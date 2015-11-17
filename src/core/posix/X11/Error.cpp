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

#include <core/posix/X11/Error.hpp>

namespace core
{
    namespace X11
    {
        Error::Error(unsigned int error_code, const char* location) noexcept
            : std::runtime_error("")
            , m_code(error_code)
            , m_what(location)
        {
            if (location == nullptr) {
                m_what = "<unknown function>";
            }
            m_what += ": ";
            m_what += std::to_string(m_code);
        }

        Error::Error(unsigned int error_code, const std::string& location) noexcept
            : Error(error_code, location.c_str())
        {
        }

        const char* Error::what() const noexcept
        {
            return m_what.c_str();
        }
    }
}
