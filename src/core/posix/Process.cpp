/*
 * Process.cpp
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


#include <core/Process.hpp>

#include <stdexcept>

namespace core
{
    struct Process::Impl
    {
        Impl(signed int pid) : m_pid(pid) {}

        bool started_by(std::string application);

    private:
        signed int m_pid;
    };

    // Normal constructors.

    Process::Process(signed int pid) : pimpl(new Impl(pid)) {}
    // Must not be called on POSIX -- PID is a signed int!
    Process::Process(unsigned int)
    {
        throw std::logic_error("Called wrong PID constructor on core::Process");
    }

    // Copy and move constructors and destructor.
    Process::Process(const Process& rhs) : pimpl(new Impl(*rhs.pimpl)) {}
    Process& Process::operator =(const Process& rhs)
    {
        if (&rhs != this) {
            *pimpl = *rhs.pimpl;
        }
        return *this;
    }
    Process::Process(Process&&) = default;
    Process& Process::operator =(Process&&) = default;
    Process::~Process() = default;

    // Forwarding.

    bool Process::started_by(const std::string& application)
    {
        return pimpl->started_by(application);
    }

    // A process has been started by an application if we look up
    // its original command line using PS and the first component
    // equals `application`.
    bool Process::Impl::started_by(const std::string& application)
    {

    }
}

