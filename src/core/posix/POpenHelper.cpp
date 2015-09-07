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


#include <core/posix/POpenHelper.hpp>

#include <stdexcept>

namespace core
{
    POpenHelper::POpenHelper() noexcept : m_out(0) {}

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
        if (m_out != nullptr) {
            close();
        }
        m_out = popen(cmdline.c_str(), "r");
        if (m_out == nullptr) {
            throw std::runtime_error("popen call to\"" + cmdline + "\" failed");
        }
    }

    std::string POpenHelper::get_output()
    {
        std::string output;
        output.reserve(250);
        char c = 0;
        while ( (c = std::fgetc(m_out)) != EOF ) {
            output.push_back(c);
        }
        return output;
    }

    int POpenHelper::close()
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

    std::string POpenHelper::check_output(const std::string& cmdline)
    {
        POpenHelper helper(cmdline);
        std::string output = helper.get_output();
        const int exit_code = helper.close();
        if (exit_code != 0) {
            throw std::runtime_error(
                "popen call \"" + cmdline + "\" failed with exit code " +
                std::to_string(exit_code)
                );
        }
        else {
            return output;
        }
    }
}
