/*
 * ConnectionImpl.cpp
 *
 * Copyright 2015 Nico <nico@FARD>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

#include <core/posix/X11/Error.hpp>
#include <core/posix/X11/ConnectionImpl.hpp>

using std::unique_ptr;

namespace core
{
    namespace X11
    {
    Connection::Impl::Impl(const char* display)
        : m_c(xcb_connect(display, nullptr))
        , m_syms(nullptr)
    {
        // Check connection.
        const int connection_error = xcb_connection_has_error(m_c);
        if (connection_error) {
            throw Error(connection_error, "xcb_connect");
        }
        // Get atoms.
        auto atoms = intern_atoms({ "_NET_WM_PID"
                                  , "_NET_WM_NAME"
                                  , "_NET_ACTIVE_WINDOW"
                                  , "_NET_CLIENT_LIST"
                                  });
        m_pid_atom = atoms[0];
        m_win_name_atom = atoms[1];
        m_active_win_atom = atoms[2];
        m_client_list_atom = atoms[3];
        // Load key symbols.
        m_syms = xcb_key_symbols_alloc(m_c);
        if (!m_syms) {
            throw Error(0, "xcb_key_symbols_alloc");
        }
        m_alt_code = get_key_code(XK_Alt_L);
        m_ctrl_code = get_key_code(XK_Control_L);
        m_shift_code = get_key_code(XK_Shift_L);
    }

    Connection::Impl::~Impl()
    {
        xcb_key_symbols_free(m_syms);
        xcb_disconnect(m_c);
    }

    xcb_window_t Connection::Impl::get_parent(xcb_window_t child) const
    {
        xcb_query_tree_cookie_t cookie = xcb_query_tree(m_c, child);
        auto reply = get_reply(xcb_query_tree_reply, cookie, "xcb_query_tree");
        return reply->parent;
    }

    bool Connection::Impl::is_descendant( xcb_window_t child
                                         , xcb_window_t parent
                                         ) const
    {
        if (child == XCB_WINDOW_NONE || parent == XCB_WINDOW_NONE) {
            return false;
        }
        do {
            if (child == parent) {
                return true;
            }
            else {
                child = get_parent(child);
            }
        } while (child != XCB_WINDOW_NONE);
        return false;
    }

    xcb_window_t Connection::Impl::get_input_focus() const noexcept
    {
        // We use the unchecked version here because the
        // X Window System Protocol guarantees that this request
        // never causes an error.
        auto cookie = xcb_get_input_focus_unchecked(m_c);
        auto reply = get_reply(xcb_get_input_focus_reply, cookie, "get_input_focus");
        return reply->focus;
    }

    xcb_window_t Connection::Impl::get_active_window() const
    {
        auto windows = get_root_windows();
        // Corner cases: 0 and 1 screens.
        // Just return `None` or the active window on the only screen.
        if (windows.size() == 1) {
            return get_active_window_by_root(windows[0]);
        }
        else if (windows.size() == 0) {
            throw Error(0, "get_active_window");
        }
        else {
            // If we got several screens, first get the active window on
            // each of them.
            for (xcb_window_t& window : windows) {
                // In-place mapping avoids allocating another vector.
                window = get_active_window_by_root(window);
            }
            // Find the one active window that is ancestor of the input focus.
            auto active_window = get_any_ancestor(windows, get_input_focus());
            if (active_window == XCB_WINDOW_NONE) {
                throw Error(0, "get_active_window");
            }
            else {
                return active_window;
            }
        }
    }

    std::vector<Window> Connection::Impl::get_top_level_windows() const
    {
        constexpr int expected_number_of_windows = 5;
        auto roots = get_root_windows();
        std::vector<Window> windows;
        for (xcb_window_t root : roots) {
            const auto buffer = get_property( root, m_client_list_atom
                                            , XCB_ATOM_ANY
                                            , expected_number_of_windows
                                            );
            const auto num_windows = buffer.size() / sizeof(xcb_window_t);
            const auto* window_p = reinterpret_cast<const xcb_window_t*>(buffer.data());
            for (size_t i=0; i<num_windows; ++i) {
                windows.push_back(*window_p);
                ++window_p;
            }
        }
        return windows;
    }

    unsigned long Connection::Impl::get_pid_window(xcb_window_t window) const
    {
        std::string buffer = get_property( window
                                         , m_pid_atom
                                         , XCB_ATOM_CARDINAL
                                         , 1
                                         );
        return *reinterpret_cast<const uint32_t*>(buffer.data());
    }

    void Connection::Impl::send_key_combo
        (const core::KeyCombo& combo
        , xcb_window_t window
        )
    {
        // For now, assume that KeyCombo's key code is an X11 keysym.
        auto keycode = get_key_code(combo.get_key_code());
        if (combo.has_ctrl()) {
            send_fake_input(window, XCB_KEY_PRESS, m_ctrl_code);
        }
        if (combo.has_shift()) {
            send_fake_input(window, XCB_KEY_PRESS, m_shift_code);
        }
        if (combo.has_alt()) {
            send_fake_input(window, XCB_KEY_PRESS, m_alt_code);
        }
        send_fake_input(window, XCB_KEY_PRESS, keycode);
        send_fake_input(window, XCB_KEY_RELEASE, keycode);
        if (combo.has_alt()) {
            send_fake_input(window, XCB_KEY_RELEASE, m_alt_code);
        }
        if (combo.has_shift()) {
            send_fake_input(window, XCB_KEY_RELEASE, m_shift_code);
        }
        if (combo.has_ctrl()) {
            send_fake_input(window, XCB_KEY_RELEASE, m_ctrl_code);
        }
        xcb_flush(m_c);
    }

    std::string Connection::Impl::get_window_title(xcb_window_t window) const
    {
        constexpr size_t title_length_estimate = 160/sizeof(uint32_t);
        return get_property(window, m_win_name_atom, XCB_ATOM_ANY, title_length_estimate);
    }

    bool Connection::Impl::window_exists(xcb_window_t window) const
    {
        // Get a property that definitely exists.
        // If there is an error, the cause is the window not existing.
        auto cookie = xcb_get_property(m_c, 0, window, XCB_ATOM_WM_NAME, 0, 0, 0);
        xcb_generic_error_t* error;
        auto* reply = xcb_get_property_reply(m_c, cookie, &error);
        if (reply) {
            // Window exists, no problem.
            free(reply);
            return true;
        }
        else {
            // Error, check the code.
            const auto error_code = error->error_code;
            free(error);
            if (error_code == XCB_WINDOW) {
                // Error caused by invalid window ID, score!
                return false;
            }
            else {
                // Unexpected error, fail loudly.
                throw Error(error_code, __func__);
            }
        }
    }

    template<typename reply_t, typename cookie_t>
    unique_ptr<reply_t> Connection::Impl::get_reply
        ( reply_getter_t<reply_t, cookie_t> raw_get_reply
        , cookie_t cookie
        , const char* fail_location
        ) const
    {
        xcb_generic_error_t* error { nullptr };
        unique_ptr<reply_t> reply { raw_get_reply(m_c, cookie, &error) };
        if (reply != nullptr) {
            return reply;
        }
        else {
            // This might happen.
            const auto error_code = error->error_code;
            free(error);
            throw Error(error_code, fail_location);
        }
    }

    std::vector<xcb_atom_t> Connection::Impl::intern_atoms
        ( const std::vector<std::string>& names
        ) const
    {
        constexpr int only_if_exists = 1;
        // Get a series of cookies.
        std::vector<xcb_intern_atom_cookie_t> cookies;
        cookies.reserve(names.size());
        for (const std::string& name : names) {
            auto cookie = xcb_intern_atom(m_c, only_if_exists, name.size(), name.c_str());
            cookies.push_back(cookie);
        }
        // Unwrap each atom.
        std::vector<xcb_atom_t> atoms;
        unique_ptr<xcb_intern_atom_reply_t> reply;
        size_t i_name = 0;
        for (const auto& cookie : cookies) {
            try {
                reply = get_reply(xcb_intern_atom_reply, cookie, "");
                atoms.push_back(reply->atom);
            }
            catch (const Error& exc) {
                std::string message = "xcb_intern_atom: " + names[i_name];
                throw Error(exc.get_error_code(), message.c_str());
            }
            ++i_name;
        }
        return atoms;
    }

    std::string Connection::Impl::get_property
        ( xcb_window_t window
        , xcb_atom_t prop
        , xcb_atom_t prop_type
        , uint32_t words_per_read
        ) const
    {
        std::string result;
        result.reserve(words_per_read * sizeof(uint32_t));
        size_t words_read = 0;

        xcb_get_property_cookie_t cookie;
        unique_ptr<xcb_get_property_reply_t> reply;
        do {
            cookie = xcb_get_property(m_c, 0, window, prop, prop_type,
                                      words_read, words_per_read);
            reply = get_reply(xcb_get_property_reply, cookie, "xcb_get_property");
            if (reply->type == XCB_ATOM_NONE ||
                (prop_type != XCB_ATOM_ANY && prop_type != reply->type))
            {
                throw Error(0, "xcb_get_property");
            }

            words_read += words_per_read;

            const size_t value_length = xcb_get_property_value_length(reply.get());
            const void* value = xcb_get_property_value(reply.get());
            result.append(static_cast<const char*>(value), value_length);

        } while (reply->bytes_after > 0);

        return result;
    }

    xcb_keycode_t Connection::Impl::get_key_code(xcb_keysym_t symbol) const
    {
        // Memoization logic.
        static xcb_keysym_t last_symbol;
        static xcb_keycode_t last_keycode;
        if (symbol == last_symbol) {
            return last_keycode;
        }
        else {
            // Retrieve key codes of an unknown symbol.
            unique_ptr<xcb_keycode_t[]> keys {
                xcb_key_symbols_get_keycode(m_syms, symbol)
                };
            if (keys == nullptr) {
                throw Error(symbol, "xcb_key_symbols_get_keycode");
            }
            // Update memo.
            last_symbol = symbol;
            last_keycode = keys[0];
            // We are only interested in the first code.
            // It may be XCB_NO_SYMBOL.
            return last_keycode;
        }
    }

    void Connection::Impl::send_fake_input
        ( xcb_window_t window
        , uint8_t type
        , xcb_keycode_t key_code
        )
    {
        xcb_test_fake_input(m_c, type, key_code, XCB_CURRENT_TIME, window, 0, 0, 0);
    }

    std::vector<xcb_window_t> Connection::Impl::get_root_windows() const noexcept
    {
        std::vector<xcb_window_t> roots;
        const auto* setup = xcb_get_setup(m_c);
        size_t num_screens = xcb_setup_roots_length(setup);

        auto screen = xcb_setup_roots_iterator(setup);
        for (size_t i = 0; i < num_screens; ++i) {
            roots.push_back(screen.data->root);
            xcb_screen_next(&screen);
        }
        return roots;
    }

    xcb_window_t Connection::Impl::get_active_window_by_root
        (xcb_window_t root
        ) const
    {
        auto buffer = get_property( root
                                  , m_active_win_atom
                                  , XCB_ATOM_WINDOW
                                  , sizeof(xcb_window_t)/sizeof(uint32_t)
                                  );
        return *reinterpret_cast<const xcb_window_t*>(buffer.data());
    }

    xcb_window_t Connection::Impl::get_any_ancestor
        ( std::vector<xcb_window_t> candidates
        , xcb_window_t child
        ) const
    {
        // Also cover the corner case that child is *in* candidates.
        while (child != XCB_WINDOW_NONE) {
            // If one of the candidates matches, return it.
            for (const auto& candidate : candidates) {
                if (child == candidate) {
                    return candidate;
                }
            }
            // Otherwise, go up one level in the ancestral chain.
            child = get_parent(child);
        }
        return XCB_WINDOW_NONE;
    }
    }
}
