/*
 * POpenHelper.hpp
 *
 * A small C++ wrapper around the ugly C-ness of popen.
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
#include <cstdio>


namespace core
{
    class POpenHelper
    {
        public:
            POpenHelper() noexcept;
            POpenHelper(const std::string& cmdline);
            ~POpenHelper();

            std::string get_output();
            void open(const std::string& cmdline);
            int close();

            static std::string check_output(const std::string& cmdline);

        private:
            std::FILE* m_out;
    };
}
