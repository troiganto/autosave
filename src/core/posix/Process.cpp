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
        //! \sa Process::Process()
        Impl(unsigned long pid) noexcept : m_pid(pid) {}
        Impl(const Impl& rhs) noexcept = default;
        Impl(Impl&& rhs) noexcept = default;
        Impl& operator =(const Impl& ehs) noexcept = default;
        Impl& operator =(Impl&& rhs) noexcept = default;
        ~Impl() = default;

        bool started_by(const std::string& application) const;

    private:
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
    /*!Implementation of Process::started_by().
     *
     * Makes one of four different attemps to find out if a process has
     * been started by the specified \a application, depending on the
     * compiler flag it has been compiled with:
     *
     * - `TEST_PROCESS_BY_PS`:
     *
     *   Use the `ps` tool to get the first fifteen characters of the
     *   application of this process.
     *   If \a application's first fifteen characters equal the ones
     *   returned by `ps`, return `true`, otherwise `false`.
     *
     * - `TEST_PROCESS_BY_CMDLINE`:
     *
     *   Compare `application` with the contents of the file
     *   /proc/<PID>/cmdline, where the file is only read up to its first
     *   null byte.
     *
     * - `TEST_PROCESS_BY_PIDOF`:
     *
     *   Call `pidof` with \a application and return `true` if this object's
     *   PID appears in the returned list, `false` otherwise.
     *
     * - if none of the above:
     *
     *   Compare the target of the link `/proc/<PID>/exe` with
     *   \a application.
     *
     * \sa Process::started_by()
     */
    bool Process::Impl::started_by(const std::string& application) const
    {
        #ifdef TEST_PROCESS_BY_PS
            POpenHelper poh("ps -p "+std::to_string(m_pid)+" -o comm=");
            std::string my_app_prefix = poh.get_output();
            return application.find(my_app_prefix) == 0;
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
                    POpenHelper poh("readlink /proc/"+std::to_string(m_pid)+"/exe");
                    const std::string my_exe = poh.get_output();
                    return application == my_exe;
                #endif
            #endif
        #endif
    }
}

