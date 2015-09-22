/*
 * XConnection.hpp
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
#include <stdexcept>
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
        /*!Class of exceptions thrown by XConnection.
         */
        class Error : public std::runtime_error
        {
        public:
            /*!Create an instance.
             *
             * \param error_code The X11 error code of the error.
             * \param location   The name of the function which caused
             *                   the error.
             */
            Error(unsigned int error_code, const char* location="") noexcept;

            virtual ~Error() = default;

            /*!Return an explanatory string.
             *
             * \returns Pointer to a null-terminated string with
             *          explanatory information.
             */
            virtual const char* what() const noexcept;

            /*!Return the X11 error code of the exception.
             *
             * \returns The X11 error code of the exception.
             */
            inline unsigned int get_error_code() const noexcept {
                return m_code;
            }
        private:
            unsigned int m_code;
            std::string m_what;
        };

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
        class XConnection
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
            XConnection(const char* display=nullptr);

            /*!Disconnect from the X server and releases all allocated
             * memory.
             */
            ~XConnection();

            /*!Return the parent of a given window.
             *
             * \param child The queried window.
             * \returns The parent window of \a child.
             * \throws X11::Error if \a child doesn't exist.
             */
            Window get_parent(Window child) const;

            /*!Return the current input focus of the display.
             *
             * \warning The input focus may and usually does differ from
             * the active window. See get_active_window() for more details.
             *
             * \returns The window currently having the input focus.
             *
             * \sa get_active_window
             */
            Window get_input_focus() const noexcept;

            /*!Return the active window as specified by `_NET_ACTIVE_WINDOW`.
             */
            Window get_active_window() const;

            unsigned long get_pid_window(Window window) const;

            void send_key_combo(const core::KeyCombo& combo);
            void send_key_combo(const core::KeyCombo& combo, Window window);

            std::string get_window_title(Window window) const;

            bool window_exists(Window window) const;

        private:
            struct Impl;
            std::unique_ptr<Impl> pimpl; //!< This class uses the PIMPL idiom.
        };
    }
}
