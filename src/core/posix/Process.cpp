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


#include "core/Process.hpp"
#include "core/posix/POpenHelper.hpp"

#include <string>
#include <fstream>
#include <stdexcept>

namespace core
{
    struct Process::Impl
    {
        Impl(unsigned long pid) noexcept : m_pid(pid) {}
        bool started_by(const std::string& application) const;

    private:
        /*!Execute a command line and get
         */
        std::string run_application(const std::string& cmdline) const;
        unsigned long m_pid; //!< The process ID of this Process.
    };



    Process::Process(unsigned long pid)
        : pimpl(std::make_unique<Impl>(pid))
    {}

    Process::Process(const Process& rhs)
        : pimpl(std::make_unique<Impl>(*rhs.pimpl))
    {}

    Process& Process::operator =(const Process& rhs)
    {
        if (&rhs != this) {
            *pimpl = *rhs.pimpl;
        }
        return *this;
    }
    Process::Process(Process&&) noexcept = default;

    Process& Process::operator =(Process&&) noexcept = default;

    Process::~Process() = default;

    bool Process::started_by(const std::string& application) const
    {
        return pimpl->started_by(application);
    }
    // A process has been started by an application if we look up
    // its original command line using PS and the first component
    // equals `application`.
    bool Process::Impl::started_by(const std::string& application) const
    {
        #ifdef TEST_PROCESS_BY_EXE
            POpenHelper poh("readlink /proc/"+std::to_string(m_pid)+"/exe");
            const std::string my_exe = poh.get_output();
            return application == my_exe;
        #else
            #ifdef TEST_PROCESS_BY_CMDLINE
                std::ifstream cmdline("/proc/"+std::to_string(m_pid)+"/cmdline");
                std::string my_application;
                std::getline(cmdline, my_application, '\0');
                return application == my_application;
            #else
                #ifdef TEST_PROCESS_BY_PIDOF
                    POpenHelper poh("pidof \""+application+"\"");
                    const std::string other_pids = poh.get_output();
                    const size_t pos = other_pids.find(std::to_string(m_pid));
                    return pos != std::string::npos;
                #else
                    POpenHelper poh("ps -p "+std::to_string(m_pid)+" -o comm=");
                    std::string my_app_prefix = poh.get_output();
                    return application.find(my_app_prefix) == 0;
                #endif
            #endif
        #endif
    }
}

