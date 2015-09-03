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

namespace core
{
    namespace Verbosity
    {
        enum Verbosity : unsigned int
        {
            MINIMUM = 0,
            QUIET = MINIMUM,
            SHOW_ICONS,
            ALERT_START,
            ALERT_FIVE_SECONDS,
            MAXIMUM = ALERT_FIVE_SECONDS
        };

        // Convert an enum to int.
        inline unsigned int to_int(Verbosity verbosity) {
            return static_cast<unsigned int>(verbosity);
        }

        // Convert int to Verbosity. Throw out_of_range error if necessary.
        Verbosity to_enum_strict(unsigned int i);

        // Convert int to Verbosity. Clamp out-of-range values to min/max.
        Verbosity to_enum(unsigned int i);
    }
}
