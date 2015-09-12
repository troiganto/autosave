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
        Window get_active_window() const noexcept;
        unsigned long get_pid_window(Window window) const;
        // Delay is measured in microseconds.
        void send_key_combo(Window window, const KeyCombo& combo);
        std::string get_window_title(Window window) const;
        bool window_exists(Window window) const;

    protected:
        // Unwrap an atom given the cookie from a previous intern_atom request.
        xcb_atom_t unwrap_atom(xcb_intern_atom_cookie_t cookie) const;

        // Get property and automatically make sure that we get the whole property.
        // Actually inefficient, but we don't care since prop_len should be a very
        // good guess.
        std::unique_ptr<xcb_get_property_reply_t>
        get_property(xcb_window_t window, xcb_atom_t prop,
                     xcb_atom_t prop_type, uint32_t prop_len) const;
        xcb_keycode_t get_key_code(xcb_keysym_t symbol) const;
        void send_fake_input(xcb_window_t window, uint8_t type, xcb_keycode_t key_code);

    private:
        xcb_connection_t* m_c;

        xcb_atom_t m_pid_atom;
        xcb_atom_t m_win_name_atom;

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

    // X11::XConnection member functions

    XConnection::XConnection(const char* display)
        : pimpl(new Impl(display))
    {}

    XConnection::~XConnection() {}

    Window XConnection::get_active_window() const noexcept
    {
        return pimpl->get_active_window();
    }

    unsigned long XConnection::get_pid_window(Window window) const
    {
        return pimpl->get_pid_window(window);
    }

    void XConnection::send_key_combo(Window window, const KeyCombo& combo)
    {
        pimpl->send_key_combo(window, combo);
    }

    std::string XConnection::get_window_title(Window window) const
    {
        return pimpl->get_window_title(window);
    }

    bool XConnection::window_exists(Window window) const
    {
        return pimpl->window_exists(window);
    }

    // X11::XConnection::Impl member functions

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
    }

    XConnection::Impl::~Impl()
    {
        xcb_key_symbols_free(m_syms);
        xcb_disconnect(m_c);
    }

    inline Window XConnection::Impl::get_active_window() const noexcept
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

    inline unsigned long XConnection::Impl::get_pid_window(Window window) const
    {
        auto reply = get_property(window, m_pid_atom, XCB_ATOM_CARDINAL, 1);
        // get_property already checked that reply is not null,
        // so we don't have to again.
        const void* value = xcb_get_property_value(reply.get());
        return *static_cast<const unsigned long*>(value);
    }

    inline void XConnection::Impl::send_key_combo( Window window
                                                 , const core::KeyCombo& combo
                                                 )
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

    std::string XConnection::Impl::get_window_title(Window window) const
    {
        auto reply = get_property(window, m_pid_atom, XCB_ATOM_CARDINAL, 1);
        // get_property already checked that reply is not null,
        // so we don't have to again.
        const void* result = xcb_get_property_value(reply.get());
        return std::string(static_cast<const char*>(result));
    }

    inline bool XConnection::Impl::window_exists(Window window) const
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

} }
