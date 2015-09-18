/*
 * Communicator.cpp
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


#include "core/Communicator.hpp"
#include "core/Process.hpp"
#include "core/posix/XConnection.hpp"

namespace core
{
    struct Communicator::Impl
    {
    public:
        Process get_active_process() const;

    private:
        X11::XConnection m_xconn;
    };

    Communicator::Communicator() : pimpl(new Impl) {}
    Communicator::Communicator(Communicator&&) = default;
    Communicator& Communicator::operator =(Communicator&&) = default;
    Communicator::~Communicator() = default;

    // Forwarding from class to its implementation.

    Process Communicator::get_active_process() const
    {
        return pimpl->get_active_process();
    }

    // Actual implementations.

    Process Communicator::Impl::get_active_process() const
    {
        const X11::Window window = m_xconn.get_active_window();
        const unsigned long pid = m_xconn.get_pid_window(window);
        return Process(pid);
    }
}
