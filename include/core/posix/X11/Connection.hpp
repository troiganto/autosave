/*
 * Connection.hpp
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

#include <string>
#include <memory>

// TODO: Maybe it'd be fancy to realize the X11 requests via C++11 futures?

namespace core
{
    // Forward declaration of KeyCombo class.
    class KeyCombo;

    /*! Namespace of X11-related classes and types.
     */
    namespace X11
    {
        /*!A type that represents an X11 window.
         *
         * The X11 namespace contains its own definition, distinct from
         * XCB, so as to not expose the C library to the outside.
         */
        typedef unsigned long Window;

        /*!Class for managing a connection to the X server and send requests.
         *
         * This class is a wrapper around calls to the XCB library.
         * Each function call results in an X11 request being sent and
         * usually blocks until a reply has arrived.
         * Errors usually result in an X11::Error being thrown.
         */
        class Connection
        {
        public:
            /*!Create an instance and initialize it.
             *
             * The constructor creates a connection to the specified
             * X11 display and fetches several necessary objects.
             * These are:
             *
             * - the atom `_NET_WM_PID`;
             * - the atom `_NET_WM_NAME`;
             * - the atom `_NET_ACTIVE_WINDOW`;
             * - the key symbol look-up table;
             * - from it, the key codes of the symbols Control_L,
             *   Shift_L, and Alt_L.
             *
             * \param display Name of the display to connect to. If not
             *                specified, connect to the default display.
             *
             * \throws X11::Error if fetching any of the entities specified
             *         above fails.
             *         If connecting to the X server or fetching the
             *         look-up table fails, the error code will be 0.
             */
            Connection(const char* display=nullptr);

            /*!Disconnect from the X server and releases all allocated
             * memory.
             */
            ~Connection();

            /*!Return the parent of a given window.
             *
             * \param child The queried window.
             * \returns The parent window of \a child.
             * \throws X11::Error if \a child doesn't exist.
             */
            Window get_parent(Window child) const;

            /*!Return \c true if \a child is a descendant of \a parent.
             *
             * \note
             * `is_ancestor(window, window) == true`
             * `is_ancestor(0, window) == false`
             * `is_ancestor(window, 0) == false`
             * `is_ancestor(0, 0) == false`
             *
             * \param parent The ancestor to be checked.
             * \param child The descendant to be checked.
             * \returns \c true if a chain of calls to get_parent() on
             *          \a child leads to \a parent, \c false otherwise.
             * \throws X11::Error if \a child doesn't exist and
             *         `parent != child`.
             */
            bool is_descendant(Window parent, Window child) const;

            /*!Return the current input focus.
             *
             * Returns the X11 window that currently has the input focus.
             *
             * \warning The input focus may and usually does differ from
             * the active window. See get_active_window() for more details.
             *
             * \returns The currently focused window.
             *
             * \sa get_active_window
             */
            Window get_input_focus() const noexcept;

            /*!Return the active window.
             *
             * Determines the active window through the following algorithm:
             *
             * 1. Query each screen of the durrent display for its active
             *    window by asking for `_NET_ACTIVE_WINDOW`.
             * 2. If the display only has one screen, return its active window.
             * 3. Otherwise, return that active window that is an ancestor
             *    of the window having the input focus.
             *
             * \returns The window that is active according to the window
             *          manager.
             *
             * \throws X11::Error if the atom `_NET_ACTIVE_WINDOW` is
             *         not supported by the window manager.
             *         In some nonsensical situations (e.g. if the display
             *         has no screens), an X11::Error with error code 0
             *         may be thrown.
             *
             * \sa get_active_window()
             */
            Window get_active_window() const;

            /*!Return the ID of the process owning the specified window.
             *
             * This function uses `_NET_WM_PID` to query the specified
             * window fow the PID of its owner process.
             *
             * \param window The window to be queried.
             *
             * \returns the PID of the owner process.
             *
             * \throws X11::Error if the window does not have the
             *         `_NET_WM_PID` property.
             *
             */
            unsigned long get_pid_window(Window window) const;

            /*!Send a series of key events to the specified window.
             *
             * This function uses \c xcb_test_fake_input() to send key
             * events.
             * It fails silently if any error occurs.
             *
             * \param combo The KeyCombo object describing the key events
             *              to be sent. Its \a key_code is expected to be
             *              an X11 key symbol.
             * \param window The window to which to send the key events.
             *               If not specified, defaults to the window
             *               which has the input focus.
             *
             * \throws X11::Error if there is no key code corresponding
             *         to the symbol in \a combo.
             *
             * \sa get_input_focus()
             */
            void send_key_combo(const core::KeyCombo& combo, Window window);

            /*! \overload
             */
            void send_key_combo(const core::KeyCombo& combo);

            /*!Return the title of the specified window.
             *
             * This function uses `_NET_WM_NAME` to query the specified
             * window fow its title.
             *
             * \param window The window to be queried.
             *
             * \returns The window title, encoded in UTF-8.
             *
             * \throws X11::Error if the window does not have the
             *         `_NET_WM_NAME` property.
             *
             */
            std::string get_window_title(Window window) const;

            /*!Return \c true if the specified window is valid.
             *
             * This function queries the specified window for the atom
             * `WM_NAME` and checks for an `XCB_WINDOW` error,
             *
             * \param window The window to be checked.
             *
             * \returns `true` if the window exists, `false` otherwise.
             *
             * \throws X11::Error if querying for `WM_NAME` gives any
             *         error other than `XCB_WINDOW`.
             */
            bool window_exists(Window window) const;

        private:
            struct Impl;
            std::unique_ptr<Impl> pimpl; //!< This class uses the PIMPL idiom.
        };
    }
}
