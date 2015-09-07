/*
 * Communicator.hpp
 *
 * A class to wrap platform-dependent code to interact with other
 * processes.
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
#include <core/Process.hpp>

namespace core
{
    class Communicator
    {
    public:
        Communicator();
        Communicator(const Communicator&) = delete;
        Communicator& operator =(const Communicator&) = delete;
        Communicator(Communicator&& rhs);
        Communicator& operator =(Communicator&& rhs);
        ~Communicator();

        Process get_active_process() const;

    private:
        // PIMPL idiom.
        class Impl;
        std::unique_ptr<Impl> pimpl;
    };

}
