/*
 * POpenHelper.cpp
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


#include "core/posix/POpenHelper.hpp"

#include <system_error>
#include <stdexcept>
#include <cerrno>
#include <cstdio>

namespace core
{
    POpenHelper::POpenHelper() noexcept
        : m_out(nullptr)
    {}

    POpenHelper::POpenHelper(const std::string& cmdline)
        : POpenHelper()
    {
        open(cmdline);
    }

    POpenHelper::~POpenHelper()
    {
        close();
    }

    void POpenHelper::open(const std::string& cmdline)
    {
        close();
        m_out = popen(cmdline.c_str(), "r");
        if (m_out == nullptr) {
            std::error_code errcode(errno, std::system_category());
            throw std::system_error(errcode, cmdline);
        }
    }

    std::string POpenHelper::get_output()
    {
        if (m_out == nullptr) {
            throw std::invalid_argument("POpenHelper uninitialized");
        }
        else {
            std::string output;
            output.reserve(250);
            char c = 0;
            while ( (c = std::fgetc(m_out)) != EOF ) {
                output.push_back(c);
            }
            return output;
        }
    }

    int POpenHelper::close() noexcept
    {
        if (m_out != nullptr) {
            const int result = pclose(m_out);
            m_out = nullptr;
            return WEXITSTATUS(result);
        }
        else {
            return 0;
        }
    }
}
