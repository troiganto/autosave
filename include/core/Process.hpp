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
    //! Platform-independent wrapper class around representations of processes.
    class Process
    {
    public:

        //! Create an object representing the calling process.
        Process();

        //! Convert a platform-dependent integer into a Process object.
        explicit Process(unsigned long pid);

        //! Copy constructor.
        Process(const Process& rhs);

        //! Copy assignment.
        Process& operator =(const Process& rhs);

        //! Move constructor.
        Process(Process&& rhs) noexcept;

        //! Move assignment.
        Process& operator =(Process&& rhs) noexcept;

        //! Destructor.
        ~Process();

        /*!Check if the Process has been started by the specified \a application.
         *
         * \param application Path to an executable that is to be checked against.
         *
         * \returns `true` if this Process has been started from the specified
         *          \a application, `false` otherwise.
         *
         * \throws std::exception if any error occurs. The exception is
         *         platform-dependent.
         */
        bool started_by(const std::string& application) const;

    private:
        // PIMPL idiom.
        struct Impl;
        std::unique_ptr<Impl> pimpl; //!< This class uses the PIMPL idiom.
    };
}
