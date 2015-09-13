/*
 * XConnection.cpp
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


#include <core/posix/XConnection.hpp>
#include <core/KeyCombo.hpp>

#include <vector>

extern "C" {
    #include <X11/keysym.h> // All key symbols as macros.
}
#include <xcb/xcb.h>
#include <xcb/xtest.h>
#include <xcb/xcb_keysyms.h>


namespace core { namespace X11
{
    // Forward declaration
    struct XConnection::Impl
    {
    public:
        Impl(const char* display=nullptr);
        ~Impl();
        xcb_window_t get_parent(xcb_window_t child) const;
        xcb_window_t get_input_focus() const noexcept;
        xcb_window_t get_active_window() const;
        unsigned long get_pid_window(xcb_window_t window) const;
        // Delay is measured in microseconds.
        void send_key_combo(const KeyCombo& combo, xcb_window_t window);
        std::string get_window_title(xcb_window_t window) const;
        bool window_exists(xcb_window_t window) const;

    protected:
        // Unwrap an atom given the cookie from a previous intern_atom request.
        xcb_atom_t unwrap_atom(xcb_intern_atom_cookie_t cookie) const;

        // Get property and automatically make sure that we get the whole property.
        // This is very inefficient only if:
        // a) the initial guess for prop_len is bad (the request will be sent a second time)
        // b) the whole property is very long (it will be requested in whole)
        std::unique_ptr<xcb_get_property_reply_t>
        get_property( xcb_window_t window
                    , xcb_atom_t prop
                    , xcb_atom_t prop_type
                    , uint32_t prop_len
                    ) const;

        xcb_keycode_t get_key_code(xcb_keysym_t symbol) const;

        void send_fake_input( xcb_window_t window
                            , uint8_t type
                            , xcb_keycode_t key_code
                            );

        std::vector<xcb_window_t> get_root_windows() const noexcept;

        xcb_window_t get_active_window_by_root(xcb_window_t root) const;

        // Go up `child`'s line of parents and return the first member of
        // `candidates` that appears in that line.
        // Return XCB_WINDOW_NONE if no candidate is ancestor of `child`.
        xcb_window_t get_any_ancestor( std::vector<xcb_window_t> candidates
                                     , xcb_window_t child
                                     ) const;

    private:
        xcb_connection_t* m_c;

        xcb_atom_t m_pid_atom;
        xcb_atom_t m_win_name_atom;
        xcb_atom_t m_active_win_atom;

        xcb_key_symbols_t* m_syms;
        xcb_keycode_t m_alt_code;
        xcb_keycode_t m_ctrl_code;
        xcb_keycode_t m_shift_code;
    };

    // X11::Error member functions

    Error::Error(unsigned int error_code, const char* location) noexcept
        : std::runtime_error("")
        , m_code(error_code)
        , m_what("X11 error caused by ")
    {
        if (location) {
            m_what += "function ";
            m_what += location;
        }
        else {
            m_what += "unknown function";
        }
        m_what += ". Error code: ";
        m_what += std::to_string(m_code);
    }

    const char* Error::what() const noexcept
    {
        return m_what.c_str();
    }

    // X11::XConnection and X11::XConnection::Impl member functions

    XConnection::XConnection(const char* display)
        : pimpl(new Impl(display))
    {}
    XConnection::Impl::Impl(const char* display)
        : m_c(xcb_connect(display, nullptr))
        , m_syms(nullptr)
    {
        // Check connection.
        if (!m_c) {
            throw Error(0, "xcb_connect");
        }
        // Send atom requests.
        xcb_intern_atom_cookie_t pid_cookie =
            xcb_intern_atom(m_c
                           , 1              // only_if_exists
                           , 11             // name_len
                           , "_NET_WM_PID"  // name
                           );
        xcb_intern_atom_cookie_t win_name_cookie =
            xcb_intern_atom(m_c, 1, 12, "_NET_WM_NAME");
        xcb_intern_atom_cookie_t active_win_cookie =
            xcb_intern_atom(m_c, 1, 18, "_NET_ACTIVE_WINDOW");
        // Load key symbols.
        m_syms = xcb_key_symbols_alloc(m_c);
        if (!m_syms) {
            throw Error(0, "xcb_key_symbols_alloc");
        }
        m_alt_code = get_key_code(XK_Alt_L);
        m_ctrl_code = get_key_code(XK_Control_L);
        m_shift_code = get_key_code(XK_Shift_L);

        // Receive atom request replies.
        m_pid_atom = unwrap_atom(pid_cookie);
        m_win_name_atom = unwrap_atom(win_name_cookie);
        m_active_win_atom = unwrap_atom(active_win_cookie);
    }

    XConnection::~XConnection() {}
    XConnection::Impl::~Impl()
    {
        xcb_key_symbols_free(m_syms);
        xcb_disconnect(m_c);
    }

    Window XConnection::get_parent(Window child) const
    {
        return pimpl->get_parent(child);
    }
    xcb_window_t XConnection::Impl::get_parent(xcb_window_t child) const
    {
        xcb_query_tree_cookie_t cookie = xcb_query_tree(m_c, child);
        xcb_generic_error_t* error;
        xcb_query_tree_reply_t* reply = xcb_query_tree_reply(m_c, cookie, &error);
        if (reply) {
            xcb_window_t parent = reply->parent;
            free(reply);
            return parent;
        }
        else {
            // This might happen.
            const int error_code = error->error_code;
            free(error);
            throw Error(error_code, "core::XConnection::get_parent");
        }
    }

    Window XConnection::get_input_focus() const noexcept
    {
        return pimpl->get_input_focus();
    }
    xcb_window_t XConnection::Impl::get_input_focus() const noexcept
    {
        // We use the unchecked version here because the
        // X Window System Protocol guarantees that this request
        // never causes an error.
        xcb_get_input_focus_cookie_t cookie = xcb_get_input_focus_unchecked(m_c);
        xcb_get_input_focus_reply_t* reply =
            xcb_get_input_focus_reply(m_c, cookie, nullptr);
        if (reply) {
            xcb_window_t focus = reply->focus;
            free(reply);
            return focus;
        }
        else {
            // Practically impossible branch.
            return 0;
        }
    }

    Window XConnection::get_active_window() const
    {
        return pimpl->get_active_window();
    }
    xcb_window_t XConnection::Impl::get_active_window() const
    {
        auto windows = get_root_windows();
        // Corner cases: 0 and 1 screens.
        // Just return `None` or the active window on the only screen.
        if (windows.size() == 1) {
            return get_active_window_by_root(windows[0]);
        }
        else if (windows.size() == 0) {
            return XCB_WINDOW_NONE;
        }
        else {
            // If we got several screens, first get the active window on
            // each of them.
            for (xcb_window_t& window : windows) {
                // In-place mapping avoids allocating another vector.
                window = get_active_window_by_root(window);
            }
            // Find the one active window that is ancestor of the input focus.
            auto focus = get_input_focus();
            return get_any_ancestor(windows, focus);
        }
    }

    unsigned long XConnection::get_pid_window(Window window) const
    {
        return pimpl->get_pid_window(window);
    }
    unsigned long XConnection::Impl::get_pid_window(xcb_window_t window) const
    {
        auto reply = get_property( window
                                 , m_pid_atom
                                 , XCB_ATOM_CARDINAL
                                 , sizeof(unsigned long)/4
                                 );
        // get_property already checked that reply is not null,
        // so we don't have to again.
        const void* value = xcb_get_property_value(reply.get());
        return *static_cast<const unsigned long*>(value);
    }

    void XConnection::send_key_combo(const core::KeyCombo& combo)
    {
        pimpl->send_key_combo(combo, XCB_SEND_EVENT_DEST_ITEM_FOCUS);
    }
    void XConnection::send_key_combo(const core::KeyCombo& combo, Window window)
    {
        pimpl->send_key_combo(combo, window);
    }
    void XConnection::Impl::send_key_combo(const core::KeyCombo& combo, xcb_window_t window)
    {
        if (combo.has_ctrl()) {
            send_fake_input(window, XCB_KEY_PRESS, m_ctrl_code);
        }
        if (combo.has_shift()) {
            send_fake_input(window, XCB_KEY_PRESS, m_shift_code);
        }
        if (combo.has_alt()) {
            send_fake_input(window, XCB_KEY_PRESS, m_alt_code);
        }
        send_fake_input(window, XCB_KEY_PRESS, combo.get_key_code());
        send_fake_input(window, XCB_KEY_RELEASE, combo.get_key_code());
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

    std::string XConnection::get_window_title(Window window) const
    {
        return pimpl->get_window_title(window);
    }
    std::string XConnection::Impl::get_window_title(xcb_window_t window) const
    {
        // Initial guess of ~160 characters for a window title
        // should be conservative enough.
        constexpr size_t title_length_estimate = 40;
        auto reply = get_property( window
                                 , m_win_name_atom
                                 , 0
                                 , title_length_estimate
                                 );
        // get_property already checked that reply is not null,
        // so we don't have to again.
        const void* result = xcb_get_property_value(reply.get());
        const char* chars = static_cast<const char*>(result);
        return std::string(chars);
    }


    bool XConnection::window_exists(Window window) const
    {
        return pimpl->window_exists(window);
    }
    bool XConnection::Impl::window_exists(xcb_window_t window) const
    {
        // Get a property that definitely exists.
        // If there is an error, the cause is the window not existing.
        xcb_get_property_cookie_t cookie =
            xcb_get_property(m_c, 0, window, XCB_ATOM_WM_NAME, 0, 0, 0);
        xcb_generic_error_t* error;
        xcb_get_property_reply_t* reply =
            xcb_get_property_reply(m_c, cookie, &error);
        if (reply) {
            // Window exists, no problem.
            free(reply);
            return true;
        }
        else {
            // Error, check the code.
            const unsigned int error_code = error->error_code;
            free(error);
            if (error_code == XCB_WINDOW) {
                // Error caused by invalid window ID, score!
                return false;
            }
            else {
                // Unexpected error, fail loudly.
                throw Error(error_code, "core::XConnection::window_exists");
            }
        }
    }

    // Implementation-only member functions

    xcb_atom_t XConnection::Impl::unwrap_atom(xcb_intern_atom_cookie_t cookie) const
    {
        xcb_generic_error_t* error;
        xcb_intern_atom_reply_t* reply =
            xcb_intern_atom_reply(m_c, cookie, &error);
        if (reply) {
            xcb_atom_t atom = reply->atom;
            free(reply);
            return atom;
        }
        else {
            const unsigned int error_code = error->error_code;
            free(error);
            throw Error(error_code, "xcb_intern_atom");
        }
    }

    std::unique_ptr<xcb_get_property_reply_t>
    XConnection::Impl::get_property( xcb_window_t window
                                   , xcb_atom_t prop
                                   , xcb_atom_t prop_type
                                   , uint32_t prop_len
                                   ) const
    {
        // Declarations.
        xcb_get_property_cookie_t cookie;
        xcb_generic_error_t* error = nullptr;
        xcb_get_property_reply_t* reply = nullptr;
        // Loop around until we are sure we got the whole property.
        while (true) {
            cookie = xcb_get_property(m_c, 0, window, prop, prop_type, 0, prop_len);
            reply = xcb_get_property_reply(m_c, cookie, &error);
            if (!reply) {
                // An error occured, abort.
                const unsigned int error_code = error->error_code;
                free(error);
                throw Error(error_code, "core::XConnection::get_property");
            }
            else if (reply->bytes_after > 0) {
                // We didn't get everything, try again.
                prop_len += reply->bytes_after;
            }
            else {
                // We got the whole property, break out.
                break;
            }
        }
        // Implicit conversion to an std::unique_ptr.
        return std::unique_ptr<xcb_get_property_reply_t>(reply);
    }

    xcb_keycode_t XConnection::Impl::get_key_code(xcb_keysym_t symbol) const
    {
        // Memoization logic.
        static xcb_keysym_t last_symbol;
        static xcb_keycode_t last_keycode;
        if (symbol == last_symbol) {
            return last_keycode;
        }
        else {
            // Retrieve key codes of an unknown symbol.
            std::unique_ptr<xcb_keycode_t[]> keys(
                xcb_key_symbols_get_keycode(m_syms, symbol));
            if (!keys) {
                throw Error(0, "xcb_key_symbols_get_keycode");
            }
            // Update memo.
            last_symbol = symbol;
            last_keycode = keys[0];
            // We are only interested in the first code.
            // It may be XCB_NO_SYMBOL.
            return last_keycode;
        }
    }

    void XConnection::Impl::send_fake_input( xcb_window_t window
                                           , uint8_t type
                                           , xcb_keycode_t key_code
                                           )
    {
        xcb_test_fake_input(m_c, type, key_code, XCB_CURRENT_TIME, window, 0, 0, 0);
    }

    std::vector<xcb_window_t> XConnection::Impl::get_root_windows() const noexcept
    {
        std::vector<xcb_window_t> roots;
        const auto* setup = xcb_get_setup(m_c);
        int num_screens = xcb_setup_roots_length(setup);

        auto screen = xcb_setup_roots_iterator(setup);
        for (int i=0; i<num_screens; ++i) {
            roots.push_back(screen.data->root);
            xcb_screen_next(&screen);
        }
        return roots;
    }

    xcb_window_t XConnection::Impl::get_active_window_by_root(xcb_window_t root) const
    {
        auto reply = get_property( root
                                 , m_active_win_atom
                                 , XCB_ATOM_WINDOW
                                 , sizeof(xcb_window_t)/4
                                 );
        // get_property already checked that reply is not null,
        // so we don't have to again.
        const void* value = xcb_get_property_value(reply.get());
        return *static_cast<const xcb_window_t*>(value);
    }

    xcb_window_t XConnection::Impl::get_any_ancestor(std::vector<xcb_window_t> candidates
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
} }
