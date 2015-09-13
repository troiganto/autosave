/*
 * Settings.cpp
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

#include <core/Settings.hpp>

#include <algorithm>
#include <utility>

namespace core
{

    Settings::Settings()
        : m_interval {300}
        , m_combo(false, true, false, 's')
        , m_verbosity {Verbosity::QUIET}
        , m_target_apps {}
        , m_cmdline {}
    {
    }

    Settings::Settings(const Settings& rhs, Settings::Mask mask)
        : Settings()
    {
        if (mask[Bits::INTERVAL]) {
            m_interval = rhs.m_interval;
        }
        if (mask[Bits::KEY_COMBO]) {
            m_combo = rhs.m_combo;
        }
        if (mask[Bits::VERBOSITY]) {
            m_verbosity = rhs.m_verbosity;
        }
        if (mask[Bits::TARGET_APPS]) {
            m_target_apps = rhs.m_target_apps;
        }
        if (mask[Bits::CMDLINE]) {
            m_cmdline = rhs.m_cmdline;
        }
    }

    Settings::Settings(Settings&& rhs, Settings::Mask mask)
        : Settings()
    {
        if (mask[Bits::INTERVAL]) {
            m_interval = rhs.m_interval;
        }
        if (mask[Bits::KEY_COMBO]) {
            m_combo = rhs.m_combo;
        }
        if (mask[Bits::VERBOSITY]) {
            m_verbosity = rhs.m_verbosity;
        }
        if (mask[Bits::TARGET_APPS]) {
            m_target_apps = std::move(rhs.m_target_apps);
        }
        if (mask[Bits::CMDLINE]) {
            m_cmdline = std::move(rhs.m_cmdline);
        }
    }

    bool operator ==(const Settings& lhs, const Settings& rhs)
    {
        return (
            lhs.m_interval == rhs.m_interval &&
            lhs.m_combo == rhs.m_combo &&
            lhs.m_verbosity == rhs.m_verbosity &&
            lhs.m_target_apps == rhs.m_target_apps &&
            lhs.m_cmdline == rhs.m_cmdline
            );
    }

    void Settings::set_interval(unsigned int rhs)
    {
        m_interval = std::min(std::max(rhs, interval::MINIMUM), interval::MAXIMUM);
    }

    void Settings::set_key_combo(KeyCombo rhs)
    {
        m_combo = rhs;
    }

    void Settings::set_verbosity(Verbosity rhs)
    {
        m_verbosity = rhs;
    }

    void Settings::set_verbosity(int rhs)
    {
        m_verbosity = to_verbosity(rhs);
    }

    bool Settings::verbosity_exceeds(Verbosity minimal) const
    {
        return m_verbosity >= minimal;
    }

}

