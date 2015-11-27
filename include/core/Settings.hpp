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
#include <core/TargetApps.hpp>


namespace core
{
    /*!This class wraps all configuration the user can apply to Autosave.
     */
    class Settings
    {
    public:
        //! The minimum allowed value of the `interval` attribute.
        constexpr static std::chrono::seconds min_interval {5};
        //! The maximum allowed value of the `interval` attribute.
        constexpr static std::chrono::seconds max_interval {24 * 60 * 60};

        //! This enum is used to specify a Settings::Mask.
        enum Bits { INTERVAL=0  //!< Represents the `interval` attribute.
                  , KEY_COMBO   //!< Represents the `key_combo` attribute.
                  , VERBOSITY   //!< Represents the `verbosity` attribute.
                  , TARGET_APPS //!< Represents the `target_apps` attribute.
                  , CMDLINE     //!< Represents the `cmdline` attribute.
                  };

        //! Bitmask that allows copying/moving only selected attributes of a Settings object.
        typedef std::bitset<CMDLINE+1> Mask;

    public:
        /*!Create an instance with its attributes set to the default.
         */
        Settings() noexcept;

        /*!Copy only selected attributes of \a rhs.
         *
         * This copy constructor only copies those attributes of \a rhs,
         * which have their respective bit set in the \a mask.
         * For all other attributes, the default value is assumed.
         *
         * \param rhs Another Settings object.
         * \param mask The mask that determines which attributes to copy.
         */
        Settings(const Settings& rhs, Mask mask);

        /*!Move only selected attributes of \a rhs.
         *
         * This move constructor only moves those attributes of \a rhs,
         * which have their respective bit set in the \a mask.
         * For all other attributes, the default value is assumed.
         *
         * \param rhs Another Settings object.
         * \param mask The mask that determines which attributes to move.
         */
        Settings(Settings&& rhs, Mask mask) noexcept;

        // Operator overloads.
        friend bool operator ==(const Settings& lhs, const Settings& rhs) noexcept;
        friend bool operator !=(const Settings& lhs, const Settings& rhs) noexcept;

        //! \returns the current value of the `interval` attribute.
        inline std::chrono::seconds interval() const noexcept { return m_interval; }

        /*!Set a new value for the `interval` attribute.
         *
         * If \a rhs lies outside the range of min_interval and max_interval,
         * it is clamped into this range.
         */
        void interval(std::chrono::seconds rhs) noexcept;

        //! \returns the current value of the `key_combo` attribute.
        inline KeyCombo key_combo() const noexcept { return m_combo; }

        //!Set a new value for the `key_combo` attribute.
        void key_combo(const KeyCombo& rhs) noexcept;

        //! \returns the current value of the `verbosity` attribute.
        inline Verbosity verbosity() const noexcept { return m_verbosity; }

        //!Set a new value for the `verbosity` attribute.
        void verbosity(Verbosity rhs) noexcept;

        //! \overload
        void verbosity(int rhs) noexcept;

        /*!Return `true` if the current verbosity is higher than \a minimal.
         *
         * \param minimal The verbosity level to check against.
         *
         * \returns `true` if the current verbosity level is equal to or
         *          higher than \a minimal, `false` otherwise.
         */
        bool verbosity_exceeds(Verbosity minimal) const noexcept;

        //! \returns a reference to the list of `target_apps`.
        inline TargetApps& target_apps() noexcept {
            return m_target_apps;
        }

        //! \returns a const reference to the list of `target_apps`.
        inline const TargetApps& target_apps() const noexcept {
            return m_target_apps;
        }

    private:
        std::chrono::seconds m_interval;  //!< Seconds after which to send input.
        KeyCombo m_combo;                 //!< Input to be sent.
        Verbosity m_verbosity;            //!< Verbosity level of Autosave.
        TargetApps m_target_apps;         //!< Apps to which to send hotkey when active.
        std::string m_cmdline;  /*!< Command line to execute. Resulting
                                 * process becomes lone target of Autosave.
                                 * Overrides m_target_apps.
                                 */
    };

    //! Two Settings objects are considered equal if all of their attributes are equal.
    bool operator ==(const Settings& lhs, const Settings& rhs) noexcept;

    //! Two Settings objects are considered unequal if any of their attributes are unequal.
    inline bool operator !=(const Settings& lhs, const Settings& rhs) noexcept {
        return !(lhs == rhs);
    }
}
