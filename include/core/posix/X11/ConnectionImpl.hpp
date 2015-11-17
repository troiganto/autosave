/*
 * ConnectionImpl.hpp
 *
 * Incomplete C++ wrapper around libxdo, mainly for automatic memory
 * management.
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

#pragma once

#include <core/KeyCombo.hpp>
#include <core/posix/X11/Connection.hpp>

extern "C" {
    #include <X11/keysym.h> // All key symbols as macros.
}
#include <xcb/xcb.h>
#include <xcb/xtest.h>
#include <xcb/xcb_keysyms.h>

#include <string>
#include <vector>
#include <memory>

namespace core
{
    // Forward declaration of KeyCombo class.
    class KeyCombo;

    /*! Namespace of X11-related classes and types.
     */
    namespace X11
    {
        //! \brief Implementation of Connection.
        struct Connection::Impl
        {
        public:
            //! \sa Connection::Connection()

            Impl(const char* display=nullptr);
            //! \sa Connection::~Connection()
            ~Impl();

            //! \sa Connection::get_parent()
            xcb_window_t get_parent(xcb_window_t child) const;

            //! \sa Connection::is_descendant()
            bool is_descendant(xcb_window_t parent, xcb_window_t child) const;

            //! \sa Connection::get_input_focus()
            xcb_window_t get_input_focus() const noexcept;

            //! \sa Connection::get_active_window()
            xcb_window_t get_active_window() const;

            //! \sa Connection::get_pid_window()
            unsigned long get_pid_window(xcb_window_t window) const;

            //! \sa Connection::send_key_combo()
            void send_key_combo(const KeyCombo& combo, xcb_window_t window);

            //! \sa Connection::get_window_title()
            std::string get_window_title(xcb_window_t window) const;

            //! \sa Connection::window_exists()
            bool window_exists(xcb_window_t window) const;

        protected:
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
            std::vector<xcb_atom_t> intern_atoms(const std::vector<std::string>& names) const;

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
            std::unique_ptr<xcb_get_property_reply_t>
            get_property( xcb_window_t window
                        , xcb_atom_t prop
                        , xcb_atom_t prop_type
                        , uint32_t prop_len
                        ) const;

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
            xcb_keycode_t get_key_code(xcb_keysym_t symbol) const;

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
            void send_fake_input( xcb_window_t window
                                , uint8_t type
                                , xcb_keycode_t key_code
                                );

            /*!Return all root windows of the current display.
             *
             * This function iterates over all screens of the display (usually 1)
             * and returns the root window of each.
             *
             * \returns A vector of the root window of each screen of the display.
             *
             * \sa Connection::get_active_window()
             */
            std::vector<xcb_window_t> get_root_windows() const noexcept;

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
            xcb_window_t get_active_window_by_root(xcb_window_t root) const;

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
    }
}
