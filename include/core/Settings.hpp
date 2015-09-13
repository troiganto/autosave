/*
 * Settings.hpp
 *
 * Settings is a class that wraps up all configurable parameters to
 * be sent to the main thread of Autosave.
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

#include <vector>
#include <string>
#include <bitset>
#include <chrono>
#include <core/KeyCombo.hpp>
#include <core/Verbosity.hpp>


namespace core
{
    // Define bounds to the interval.
    namespace interval
    {
        // Minimum interval is 5 seconds, maximum is 1 day.
        constexpr std::chrono::seconds MINIMUM {5};
        constexpr std::chrono::seconds MAXIMUM {24 * 60 * 60};
    }

    class Settings
    {
    public:
        // Define  a type with which one may signal which of the
        // settings should be used or have been modified.
        enum Bits
        {
            INTERVAL = 0,
            KEY_COMBO,
            VERBOSITY,
            TARGET_APPS,
            CMDLINE
        };
        typedef std::bitset<CMDLINE+1> Mask;

    public:
        // Constructors, destructor.
        Settings();
        Settings(const Settings& rhs) = default;
        Settings(const Settings& rhs, Mask mask);
        Settings(Settings&& rhs) = default;
        Settings(Settings&& rhs, Mask mask);
        ~Settings() {}

        // Operator overloads.
        friend bool operator ==(const Settings& lhs, const Settings& rhs);
        friend bool operator !=(const Settings& lhs, const Settings& rhs);

        // Setter functions.
        void set_interval(std::chrono::seconds rhs);
        void set_key_combo(KeyCombo rhs);
        void set_verbosity(Verbosity rhs);
        void set_verbosity(int rhs);

        // Getter functions.
        inline std::chrono::seconds get_interval() const { return m_interval; }
        inline KeyCombo get_key_combo() const { return m_combo; }
        inline Verbosity get_verbosity() const { return m_verbosity; }

        // Extended getter functions.
        bool verbosity_exceeds(Verbosity minimal) const;

    private:
        std::chrono::seconds m_interval;        // Seconds after which to send input.
        KeyCombo m_combo;                       // Input to be sent.
        Verbosity m_verbosity;                  // Verbosity level of Autosave.
        std::vector<std::string> m_target_apps; // Apps to which to send hotkey when active.
        std::string m_cmdline;                  // Command line to execute, resulting process
                                                // becomes lone target of Autosave.
                                                // Overrides m_target_apps.
    };

    bool operator ==(const Settings& lhs, const Settings& rhs);
    inline bool operator !=(const Settings& lhs, const Settings& rhs) {
        return !(lhs == rhs);
    }
}
