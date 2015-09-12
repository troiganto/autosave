/*
 * KeyCombo.hpp
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
    class KeyCombo
    {
        // Define sub-types.
        enum Modifiers
        {
            CTRL = 0,
            ALT = 1,
            SHIFT = 2
        };
        typedef std::bitset<3> ModifierMask;

    public:
        KeyCombo(bool alt, bool ctrl, bool shift, unsigned int keycode)
            : m_modifiers(), m_keycode(keycode)
        {
            m_modifiers[Modifiers::ALT] = alt;
            m_modifiers[Modifiers::CTRL] = ctrl;
            m_modifiers[Modifiers::SHIFT] = shift;
        }
        KeyCombo() : KeyCombo(false, false, false, 0) {}
        KeyCombo(const KeyCombo& rhs) = default;
        KeyCombo(KeyCombo&& rhs) = default;
        KeyCombo& operator =(const KeyCombo& rhs) = default;
        KeyCombo& operator =(KeyCombo&& rhs) = default;

        inline bool has_alt() const { return m_modifiers[Modifiers::ALT]; }
        inline bool has_ctrl() const { return m_modifiers[Modifiers::CTRL]; }
        inline bool has_shift() const { return m_modifiers[Modifiers::SHIFT]; }
        inline unsigned int get_key_code() const { return m_keycode; }

        // Operator overloads.
        friend bool operator ==(const KeyCombo& lhs, const KeyCombo& rhs);
        friend bool operator !=(const KeyCombo& lhs, const KeyCombo& rhs);

    private:
        // Define fields.
        ModifierMask m_modifiers;
        unsigned int m_keycode;
    };

    inline bool operator ==(const KeyCombo& lhs, const KeyCombo& rhs) {
        return lhs.m_keycode == rhs.m_keycode && lhs.m_modifiers == rhs.m_modifiers;
    }
    inline bool operator !=(const KeyCombo& lhs, const KeyCombo& rhs) {
        return !(lhs == rhs);
    }
}

