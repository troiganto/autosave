/*
 * Hotkey.hpp
 *
 * Generic hotkey class which may be translated by platform-dependent code.
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

#include <bitset>

namespace core
{
    struct Hotkey
    {
        // Define sub-types.
        enum Modifiers
        {
            PRIMARY = 0,
            ALT = 1,
            SHIFT = 2
        };
        typedef std::bitset<3> ModifierMask;

        // Operator overloads.
        friend bool operator ==(const Hotkey& lhs, const Hotkey& rhs);
        friend bool operator !=(const Hotkey& lhs, const Hotkey& rhs);

        // Define fields.
        char key;
        ModifierMask modifiers;
    };

    inline bool operator ==(const Hotkey& lhs, const Hotkey& rhs) {
        return lhs.key == rhs.key && lhs.modifiers == rhs.modifiers;
    }
    inline bool operator !=(const Hotkey& lhs, const Hotkey& rhs) {
        return !(lhs == rhs);
    }
}

