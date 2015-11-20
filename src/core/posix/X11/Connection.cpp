/*
 * Connection.cpp
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


#include <core/posix/X11/Connection.hpp>
#include <core/posix/X11/ConnectionImpl.hpp>
#include <core/KeyCombo.hpp>

namespace core
{
    namespace X11
    {
        Connection::Connection(const char* display)
            : pimpl(new Impl(display))
        {}

        Connection::~Connection() {}

        Window Connection::get_parent(Window child) const
        {
            return pimpl->get_parent(child);
        }

        bool Connection::is_descendant(Window child, Window parent) const
        {
            return pimpl->is_descendant(child, parent);
        }

        Window Connection::get_input_focus() const noexcept
        {
            return pimpl->get_input_focus();
        }

        Window Connection::get_active_window() const
        {
            return pimpl->get_active_window();
        }

        std::vector<Window> Connection::get_top_level_windows() const
        {
            return pimpl->get_top_level_windows();
        }

        unsigned long Connection::get_pid_window(Window window) const
        {
            return pimpl->get_pid_window(window);
        }

        void Connection::send_key_combo(const core::KeyCombo& combo, Window window)
        {
            pimpl->send_key_combo(combo, window);
        }

        void Connection::send_key_combo(const core::KeyCombo& combo)
        {
            pimpl->send_key_combo(combo, XCB_SEND_EVENT_DEST_ITEM_FOCUS);
        }

        std::string Connection::get_window_title(Window window) const
        {
            return pimpl->get_window_title(window);
        }

        bool Connection::window_exists(Window window) const
        {
            return pimpl->window_exists(window);
        }
    }
}
