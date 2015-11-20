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
#include "core/posix/X11/Connection.hpp"

#include <algorithm>
#include <functional>

namespace core
{
    struct Communicator::Impl
    {
    public:
        bool active_window_matches(const std::vector<std::string>& target_apps) const;
        bool any_window_matches(const std::vector<std::string>& target_apps) const;
        void send(const KeyCombo& key_combo);

    private:
        Process get_active_process() const;

        X11::Connection m_xconn;
    };

    Communicator::Communicator() : pimpl(new Impl) {}
    Communicator::Communicator(Communicator&&) = default;
    Communicator& Communicator::operator =(Communicator&&) = default;
    Communicator::~Communicator() = default;

    // Forwarding from class to its implementation.

    bool Communicator::active_window_matches
        ( const std::vector<std::string>& target_apps
        ) const
    {
        return pimpl->active_window_matches(target_apps);
    }

    bool Communicator::any_window_matches
        ( const std::vector<std::string>& target_apps
        ) const
    {
        return pimpl->any_window_matches(target_apps);
    }

    void Communicator::send(const KeyCombo& key_combo)
    {
        pimpl->send(key_combo);
    }

    // Actual implementations.

    bool Communicator::Impl::active_window_matches
        ( const std::vector<std::string>& target_apps
        ) const
    {
        Process active_process = get_active_process();
        auto started_by = std::bind( &Process::started_by
                                   , active_process, std::placeholders::_1
                                   );
        return std::any_of(target_apps.begin(), target_apps.end(), started_by);
    }

    bool Communicator::Impl::any_window_matches
        ( const std::vector<std::string>& target_apps
        ) const
    {
        return false;
    }

    void Communicator::Impl::send(const KeyCombo& key_combo)
    {
        m_xconn.send_key_combo(key_combo);
    }

    Process Communicator::Impl::get_active_process() const
    {
        const X11::Window window = m_xconn.get_active_window();
        const unsigned long pid = m_xconn.get_pid_window(window);
        return Process(pid);
    }
}
