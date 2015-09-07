/*
 * Process.hpp
 *
 * An OS-independent representation of other processes.
 * It's only important feature is its ability to be compared
 * to other processes.
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

#include <memory>

namespace core
{
    class Process
    {
    public:
        Process();
        explicit Process(signed int pid);
        explicit Process(unsigned int pid);
        Process(const Process& rhs);
        Process& operator =(const Process& rhs);
        Process(Process&& rhs);
        Process& operator =(Process&& rhs);
        ~Process();

        // What the string application actually is, is platform-dependent.
        // It could be a command line, a path to an executable, or
        // an UUID. We just don't know.
        bool started_by(const std::string& application) const;

    private:
        // PIMPL idiom.
        struct Impl;
        std::unique_ptr<Impl> pimpl;
    };
}
