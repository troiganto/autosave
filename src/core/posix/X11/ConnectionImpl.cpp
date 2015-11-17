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

namespace core
{
    namespace X11
    {
    //! \sa Connection::Connection()
    Connection::Impl::Impl(const char* display)
        : m_c(xcb_connect(display, nullptr))
        , m_syms(nullptr)
    {
        // Check connection.
        int connection_error = xcb_connection_has_error(m_c);
        if (connection_error) {
            throw Error(connection_error, "xcb_connect");
        }
        // Get atoms.
        auto atoms = intern_atoms({ "_NET_WM_PID"
                                  , "_NET_WM_NAME"
                                  , "_NET_ACTIVE_WINDOW"
                                  });
        m_pid_atom = atoms[0];
        m_win_name_atom = atoms[1];
        m_active_win_atom = atoms[2];
        // Load key symbols.
        m_syms = xcb_key_symbols_alloc(m_c);
        if (!m_syms) {
            throw Error(0, "xcb_key_symbols_alloc");
        }
        m_alt_code = get_key_code(XK_Alt_L);
        m_ctrl_code = get_key_code(XK_Control_L);
        m_shift_code = get_key_code(XK_Shift_L);
    }

    //! \sa Connection::~Connection()
    Connection::Impl::~Impl()
    {
        xcb_key_symbols_free(m_syms);
        xcb_disconnect(m_c);
    }

    //! \sa Connection::get_parent()
    xcb_window_t Connection::Impl::get_parent(xcb_window_t child) const
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
            throw Error(error_code, "xcb_query_tree");
        }
    }

    //! \sa Connection::is_descendant()
    bool Connection::Impl::is_descendant( xcb_window_t parent
                                         , xcb_window_t child
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

    //! \sa Connection::get_input_focus()
    xcb_window_t Connection::Impl::get_input_focus() const noexcept
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

    //! \sa Connection::get_active_window()
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

    //! \sa Connection::get_pid_window()
    unsigned long Connection::Impl::get_pid_window(xcb_window_t window) const
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

    //! \sa Connection::send_key_combo()
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

    //! \sa Connection::get_window_title()
    std::string Connection::Impl::get_window_title(xcb_window_t window) const
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

    //! \sa Connection::window_exists()
    bool Connection::Impl::window_exists(xcb_window_t window) const
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
                throw Error(error_code, __func__);
            }
        }
    }

    /*!Wrapper around `xcb_intern_atom`.
     *
     * Takes a series of atom names and returns a series of atoms.
     *
     * \param names A vector of names to be looked up.
     *
     * \returns A series of atoms with the same ordering as in \a names.
     *
     * \throws X11::Error if any atom does not exist.
     */
    std::vector<xcb_atom_t> Connection::Impl::intern_atoms
        ( const std::vector<std::string>& names
        ) const
    {
        constexpr int only_if_exists = 1;
        // Get a series of cookies.
        std::vector<xcb_intern_atom_cookie_t> cookies;
        cookies.reserve(names.size());
        for (const std::string& name : names) {
            xcb_intern_atom_cookie_t cookie =
                xcb_intern_atom(m_c, only_if_exists, name.size(), name.c_str());
            cookies.push_back(cookie);
        }
        // Unwrap each atom.
        std::vector<xcb_atom_t> atoms;
        xcb_generic_error_t* error;
        xcb_intern_atom_reply_t* reply;
        size_t i_name = 0;
        for (const xcb_intern_atom_cookie_t& cookie : cookies) {
            reply = xcb_intern_atom_reply(m_c, cookie, &error);
            if (reply) {
                atoms.push_back(reply->atom);
                free(reply);
            }
            else {
                const unsigned int error_code = error->error_code;
                free(error);
                std::string message = "xcb_intern_atom: " + names[i_name];
                throw Error(error_code, message.c_str());
            }
            // The overhead is acceptable because this function is
            // caled only once.
            ++i_name;
        }
        return atoms;
    }

    /*!Wrapper around `xcb_get_property`.
     *
     * Wraps requesting a property, waiting for the reply, and handling it.
     *
     * \param windw     The window being queried for a property.
     * \param prop      The atom describing the property that is asked for.
     * \param prop_type The atom describing the type of the requested property.
     * \param prop_len  A hint for the expected length of the property in
     *                  words. A word usually equals 4 bytes.
     *                  If the expected length is too small, another request
     *                  has to be sent, so it's usually reasonable to give
     *                  a generous estimate.
     *
     * \returns Unique pointer to the reply structure.
     *
     * \throws X11::Error if any X11 error occurs.
     */
    std::unique_ptr<xcb_get_property_reply_t> Connection::Impl::get_property
        ( xcb_window_t window
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
                throw Error(error_code, "xcb_get_property");
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
        // Conversion to an std::unique_ptr.
        return std::unique_ptr<xcb_get_property_reply_t>(reply);
    }

    /*!Look up and return the key code corresponding to a key symbol.
     *
     * Because looking up a symbol might be slow, this function memoizes
     * the last symbol that has been looked up.
     * Thus, looking up the same symbol again and again is very efficient.
     *
     * \param symbol the key symbol to be looked up.
     *
     * \returns The first matching key code or `XCB_NO_SYMBOL` if there is
     *          none.
     *
     * \throws X11::Error with its error code set to \a symbol if the
     *         look-up fails.
     */
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
            std::unique_ptr<xcb_keycode_t[]> keys(
                xcb_key_symbols_get_keycode(m_syms, symbol));
            if (!keys) {
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

    /*!Wrapper around `xcb_test_fake_input`.
     *
     * Send fake keyboard events via the XTest library.
     * If any error occurs, this function fails silently.
     *
     * \param window The window to receive the events.
     * \param type Either `XCB_KEY_PRESS` or `XCB_KEY_RELEASE`.
     * \param key_code The code of the key which is being pressed or released.
     *
     */
    void Connection::Impl::send_fake_input
        ( xcb_window_t window
        , uint8_t type
        , xcb_keycode_t key_code
        )
    {
        xcb_test_fake_input(m_c, type, key_code, XCB_CURRENT_TIME, window, 0, 0, 0);
    }

    /*!Return all root windows of the current display.
     *
     * This function iterates over all screens of the display (usually 1)
     * and returns the root window of each.
     *
     * \returns A vector of the root window of each screen of the display.
     *
     * \sa Connection::get_active_window()
     */
    std::vector<xcb_window_t> Connection::Impl::get_root_windows() const noexcept
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

    /*!Return the active window under a given root window.
     *
     * This function queries the passed window for `_NET_ACTIVE_WINDOW`
     * and returns the result.
     *
     * \param root A root window.
     *
     * \returns The active window according to the `_NET_ACTIVE_WINDOW`
     *          property.
     *
     * \throws X11::Error if the passed window does not have the
     *         `_NET_ACTIVE_WINDOW` property.
     *
     * \sa Connection::get_active_window()
     */
    xcb_window_t Connection::Impl::get_active_window_by_root
        (xcb_window_t root
        ) const
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

    /*!Under a list of candidates, return the first that is found to be
     * an ancestor of \a child.
     *
     * This function finds the ancestor of \a child through repeated calls
     * to get_parent().
     * This means that if both a window and its child window are elements
     * of \a candidates, the parent window can never be returned by this
     * function because the child is always found first.
     *
     * \param candidates A vector of candidate windows that may or may not
     *                   be ancestors of \a child.
     * \param child      The window whose ancestor is required.
     *
     * \returns The most direct ancestor of \a child that is an element
     *          of \a candidates or `XCB_WINDOW_NONE` if no element of
     *          \a candidates is an ancestor of \a child.
     *
     * \throws X11::Error if any call to get_parent() fails.
     *
     * \sa Connection::get_active_window(), Connection::get_parent()
     */
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