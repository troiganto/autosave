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
    public:
        KeyCombo(bool alt, bool ctrl, bool shift, unsigned int keycode)
            : m_alt(alt), m_ctrl(ctrl), m_shift(shift)
            , m_keycode(keycode)
        {}
        explicit KeyCombo(unsigned int keycode)
            : KeyCombo(false, false, false, keycode)
        {}
        KeyCombo() : KeyCombo(false, false, false, 0) {}

        KeyCombo(const KeyCombo& rhs) = default;
        KeyCombo(KeyCombo&& rhs) = default;
        KeyCombo& operator =(const KeyCombo& rhs) = default;
        KeyCombo& operator =(KeyCombo&& rhs) = default;

        inline bool has_alt() const { return m_alt; }
        inline bool has_ctrl() const { return m_ctrl; }
        inline bool has_shift() const { return m_shift; }
        inline unsigned int get_key_code() const { return m_keycode; }

        // Operator overloads.
        friend bool operator ==(const KeyCombo& lhs, const KeyCombo& rhs);
        friend bool operator !=(const KeyCombo& lhs, const KeyCombo& rhs);

    private:
        // Define fields.
        bool m_alt;
        bool m_ctrl;
        bool m_shift;
        unsigned int m_keycode;
    };

    inline bool operator ==(const KeyCombo& lhs, const KeyCombo& rhs) {
        return lhs.m_keycode == rhs.m_keycode
            && lhs.m_alt == rhs.m_alt
            && lhs.m_ctrl == rhs.m_ctrl
            && lhs.m_shift == rhs.m_shift
            ;
    }
    inline bool operator !=(const KeyCombo& lhs, const KeyCombo& rhs) {
        return !(lhs == rhs);
    }
}

