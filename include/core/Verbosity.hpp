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
    enum Verbosity{ QUIET
                  , COUNTDOWN_ICONS
                  , ALERT_START
                  , ALERT_FIVE_SEC
                  , MINIMUM = QUIET
                  , MAXIMUM = ALERT_FIVE_SEC
                  };

    constexpr Verbosity to_verbosity(int i)
    {
        return (i < static_cast<int>(Verbosity::MINIMUM)) ? Verbosity::MINIMUM :
               (i > static_cast<int>(Verbosity::MAXIMUM)) ? Verbosity::MAXIMUM :
               static_cast<Verbosity>(i);
    }

    constexpr Verbosity to_verbosity_strict(int i)
    {
        return (i < static_cast<int>(Verbosity::MINIMUM) ||
                i > static_cast<int>(Verbosity::MAXIMUM)) ?
            throw std::out_of_range("to_verbosity") :
            static_cast<Verbosity>(i);
    }

    constexpr int from_verbosity(Verbosity v)
    {
        return static_cast<int>(v);
    }
}
