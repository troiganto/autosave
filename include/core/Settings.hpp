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

        /*!Copy attributes of \a rhs.
         */
        Settings(const Settings& rhs) = default;

        /*!Copy only selected attributes of \a rhs.
         *
         * This copy constructor only copies those attributes of \a rhs,
         * which have their respective bit set in the \a mask.
         * For all other attributes, the default value is assumed.
         *
         * \param rhs Another Settings object.
         * \param mask The mask that determines which attributes to copy.
         */
        Settings(const Settings& rhs, Mask mask) noexcept;

        /*! Move attributes of \a rhs.
         */
        Settings(Settings&& rhs) = default;

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

        /*!Destructor.
         */
        ~Settings() {}

        // Operator overloads.
        friend bool operator ==(const Settings& lhs, const Settings& rhs) noexcept;
        friend bool operator !=(const Settings& lhs, const Settings& rhs) noexcept;

        /*!Set a new value for the `interval` attribute.
         *
         * If \a rhs lies outside the range of min_interval and max_interval,
         * it is clamped into this range.
         */
        void set_interval(std::chrono::seconds rhs);

        //!Set a new value for the `key_combo` attribute.
        void set_key_combo(const KeyCombo& rhs);

        //!Set a new value for the `verbosity` attribute.
        void set_verbosity(Verbosity rhs);

        //! \overload
        void set_verbosity(int rhs);

        //! Return the current value of the `interval` attribute.
        inline std::chrono::seconds get_interval() const { return m_interval; }

        //! Return the current value of the `key_combo` attribute.
        inline KeyCombo get_key_combo() const { return m_combo; }

        //! Return the current value of the `verbosity` attribute.
        inline Verbosity get_verbosity() const { return m_verbosity; }

        /*!Return `true` if the current verbosity is higher than \a minimal.
         *
         * \param minimal The verbosity level to check against.
         *
         * \returns `true` if the current verbosity level is equal to or
         *          higher than \a minimal, `false` otherwise.
         */
        bool verbosity_exceeds(Verbosity minimal) const noexcept;

    private:
        std::chrono::seconds m_interval;        //!< Seconds after which to send input.
        KeyCombo m_combo;                       //!< Input to be sent.
        Verbosity m_verbosity;                  //!< Verbosity level of Autosave.
        std::vector<std::string> m_target_apps; //!< Apps to which to send hotkey when active.
        std::string m_cmdline;                  /*!< Command line to execute. Resulting process
                                                 * becomes lone target of Autosave.
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
