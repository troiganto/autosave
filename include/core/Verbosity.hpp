/*
 * Verbosity.hpp
 *
 * A scoped enum that lists all possible verbosity values of Autosave.
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

#include <stdexcept>

namespace core
{
    /*!Enum of allowed levels of verbosity for Autosave.
     */
    enum Verbosity{ QUIET                       //!< Be as quiet as possible.
                  , COUNTDOWN_ICONS             //!< Animate the sys tray icon.
                  , ALERT_START                 //!< Notify the user when starting the background thread..
                  , ALERT_FIVE_SEC              //!< Notify the user 5 seconds before a key event is sent.
                  , MINIMUM = QUIET             //!< Minimum verbosity.
                  , MAXIMUM = ALERT_FIVE_SEC    //!< Maximum verbosity.
                  };

    /*!Convert an integer to a verbosity level.
     *
     * If \a i is out of range, it is set to the nearest allowed value.
     *
     * \param i The integer to be converted.
     *
     * \returns The corresponding verbosity level. If \a i is out of range,
     *          either Verbosity::MINIMUM or Verbosity::MAXIMUM, depending
     *          on whether \a i is too high or too low.
     */
    constexpr Verbosity to_verbosity(int i) noexcept
    {
        return (i < static_cast<int>(Verbosity::MINIMUM)) ? Verbosity::MINIMUM :
               (i > static_cast<int>(Verbosity::MAXIMUM)) ? Verbosity::MAXIMUM :
               static_cast<Verbosity>(i);
    }

    /*!Convert an integer to a verbosity level.
     *
     * If \a i is out of range, an exception is thrown.
     *
     * \param i The integer to be converted.
     *
     * \returns The corresponding verbosity level.
     *
     * \throws std::out_of_range if \a i is below Verbosity::MINIMUM or
     *         above Verbosity::MAXIMUM.
     */
    constexpr Verbosity to_verbosity_strict(int i)
    {
        return (i < static_cast<int>(Verbosity::MINIMUM) ||
                i > static_cast<int>(Verbosity::MAXIMUM)) ?
            throw std::out_of_range("to_verbosity") :
            static_cast<Verbosity>(i);
    }

    /*!Convert a verbosity level to the corresponding integer.
     *
     * \param v The verbosity level to be converted.
     *
     * \returns The corresponding integer.
     */
    constexpr int from_verbosity(Verbosity v) noexcept
    {
        return static_cast<int>(v);
    }
}
