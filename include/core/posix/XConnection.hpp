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

namespace core
{
    // Forward declaration of KeyCombo class.
    class KeyCombo;

    namespace X11
    {
        class Error : public std::runtime_error
        {
        public:
            Error(unsigned int error_code, const char* location="") noexcept;
            virtual ~Error() = default;
            virtual const char* what() const noexcept;
            inline unsigned int get_error_code() const noexcept {
                return m_code;
            }
        private:
            unsigned int m_code;
            std::string m_what;
        };

        // Avoid exposing the C libraries in the header by declaring
        // our own typedefs.
        typedef unsigned long Window;
        typedef unsigned long Atom;

        class XConnection
        {
            public:
                XConnection(const char* display=nullptr);
                ~XConnection();

                Window get_active_window() const noexcept;

                unsigned long get_pid_window(Window window) const;

                // Delay is measured in microseconds.
                void send_key_combo(Window window, const core::KeyCombo& combo);

                std::string get_window_title(Window window) const;

                bool window_exists(Window window) const;

            private:
                struct Impl;
                std::unique_ptr<Impl> pimpl;
        };
    }
}
