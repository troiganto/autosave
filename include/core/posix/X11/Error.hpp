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

namespace core
{
    namespace X11
    {
        /*!Class of exceptions thrown by X11::Connection.
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

            //! \overload
            Error(unsigned int error_code, const std::string& location) noexcept;

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
    }
}
