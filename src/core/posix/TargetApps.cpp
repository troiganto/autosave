/*
 * TargetApps.cpp
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


#include "core/TargetApps.hpp"
#include "core/Process.hpp"
#include "core/posix/X11/Connection.hpp"

#include <list>
#include <algorithm>
#include <functional>

// Static XConnection -- Either fails to initialize before main() or
// it's there the whole time.
static core::X11::Connection x_connection;

namespace core
{
    using namespace std;

    struct TargetApps::Impl
    {
    public:
        Impl();
        Impl(initializer_list<string> apps);

        bool any_window_matches() const;
        bool active_window_matches() const;
        void send(const KeyCombo& key_combo);

        bool operator ==(const Impl& rhs) const noexcept
        {
            return m_apps == rhs.m_apps;
        }

        bool operator !=(const Impl& rhs) const noexcept
        {
            return m_apps != rhs.m_apps;
        }

    private:
        bool window_matches(X11::Window window) const;

        list<string> m_apps;
    };

    // Forwarding from class to its implementation.

    TargetApps::TargetApps()
        : pimpl(make_unique<Impl>())
    {}

    TargetApps::TargetApps(initializer_list<string> target_apps)
        : pimpl(make_unique<Impl>(target_apps))
    {}

    TargetApps::TargetApps(const TargetApps& rhs)
        : pimpl(make_unique<Impl>(*rhs.pimpl))
    {}

    TargetApps& TargetApps::operator =(const TargetApps& rhs)
    {
        if(&rhs != this) {
            *pimpl = *rhs.pimpl;
        }
        return *this;
    }

    TargetApps::TargetApps(TargetApps&&) = default;
    TargetApps& TargetApps::operator =(TargetApps&&) = default;

    TargetApps::~TargetApps() = default;

    bool TargetApps::active_window_matches() const
    {
        return pimpl->active_window_matches();
    }

    bool TargetApps::any_window_matches() const
    {
        return pimpl->any_window_matches();
    }

    void TargetApps::send(const KeyCombo& key_combo) const
    {
        pimpl->send(key_combo);
    }

    bool operator ==(const TargetApps& lhs, const TargetApps& rhs) noexcept
    {
        return *lhs.pimpl == *rhs.pimpl;
    }

    bool operator !=(const TargetApps& lhs, const TargetApps& rhs) noexcept
    {
        return *lhs.pimpl != *rhs.pimpl;
    }

    // Actual implementations.

    TargetApps::Impl::Impl()
        : m_apps()
    {}

    TargetApps::Impl::Impl(initializer_list<string> apps)
        : m_apps(apps)
    {}

    void TargetApps::Impl::send(const KeyCombo& key_combo)
    {
        x_connection.send_key_combo(key_combo);
    }

    bool TargetApps::Impl::any_window_matches() const
    {
        const auto windows = x_connection.get_top_level_windows();
        return any_of(windows.begin(), windows.end(), [this](auto w) {
            return this->window_matches(w);
        });
    }

    bool TargetApps::Impl::active_window_matches() const
    {
        return window_matches(x_connection.get_active_window());
    }

    bool TargetApps::Impl::window_matches(X11::Window window) const
    {
        Process process(x_connection.get_pid_window(window));
        return any_of(m_apps.begin(), m_apps.end(), [&process](const auto& app) {
            return process.started_by(app);
        });
    }
}
