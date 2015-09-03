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
#include <core/Hotkey.hpp>
#include <core/Verbosity.hpp>


namespace core
{
    // Define bounds to the interval.
    namespace interval
    {
        // Minimum interval is 5 seconds, maximum is 1 day.
        constexpr unsigned int MINIMUM = 5;
        constexpr unsigned int MAXIMUM = 24 * 60 * 60;
    }

    class Settings
    {
    public:
        // Define  a type with which one may signal which of the
        // settings should be used or have been modified.
        enum Bits
        {
            INTERVAL = 0,
            HOTKEY,
            VERBOSITY,
            TARGET_APPS,
            CMDLINE
        };
        typedef std::bitset<5> Mask;

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
        void set_interval(unsigned int rhs);
        void set_hotkey(Hotkey rhs);
        void set_verbosity(Verbosity::Verbosity rhs);
        void set_verbosity(unsigned int rhs);

        // Getter functions.
        inline unsigned int get_interval() const { return m_interval; }
        inline Hotkey get_hotkey() const { return m_hotkey; }
        inline Verbosity::Verbosity get_verbosity() const { return m_verbosity; }

        // Extended getter functions.
        bool verbosity_exceeds(Verbosity::Verbosity minimal) const;

    private:
        unsigned int m_interval;                // Seconds after which to send input.
        Hotkey m_hotkey;                        // Input to be sent.
        Verbosity::Verbosity m_verbosity;       // Verbosity level of Autosave.
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
