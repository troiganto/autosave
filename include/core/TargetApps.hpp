/*
 * TargetApps.hpp
 *
 * A class to wrap platform-dependent code to interact with other
 * processes.
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

#include <memory>
#include <initializer_list>
#include <core/KeyCombo.hpp>

namespace core
{
    class TargetApps
    {
    public:
        TargetApps();
        TargetApps(std::initializer_list<std::string> target_apps);
        TargetApps(const TargetApps&);
        TargetApps& operator =(const TargetApps&);
        TargetApps(TargetApps&&);
        TargetApps& operator =(TargetApps&&);
        ~TargetApps();

        bool active_window_matches() const;
        bool any_window_matches() const;
        void send(const KeyCombo& key_combo) const;

        friend bool operator ==(const TargetApps& lhs, const TargetApps& rhs) noexcept;
        friend bool operator !=(const TargetApps& lhs, const TargetApps& rhs) noexcept;

    private:
        // PIMPL idiom.
        class Impl;
        std::unique_ptr<Impl> pimpl;
    };
}
