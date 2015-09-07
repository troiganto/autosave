/*
 * XDo.cpp
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


#include <core/posix/XDo.hpp>

extern "C" {
    #include <xdo.h>
}

namespace core
{
    XDo::XDo(const char* display)
    {
        m_context = xdo_new(display);
        if (m_context == nullptr) {
            throw XDoError("xdo_new failed");
        }
    }

    XDo::~XDo()
    {
        if (m_context != nullptr) {
            xdo_free(m_context);
            m_context = nullptr;
        }
    }

    Window XDo::get_active_window() const
    {
        // Declare X11 window.
        ::Window window;
        const int ret_code = xdo_get_active_window(m_context, &window);
        if (ret_code) {
            throw core::XDoError("xdo_get_active_window failed");
        }
        else {
            return static_cast<Window>(window);
        }
    }

    int XDo::get_pid_window(Window window) const
    {
        // Convert from core::XDo::Window to X11 window.
        int pid = xdo_get_pid_window(m_context,
                                     static_cast<::Window>(window));
        if (!pid) {
            throw XDoError("xdo_get_pid_window failed");
        }
        else {
            return pid;
        }
    }

    std::pair<unsigned int, unsigned int>
    XDo::get_window_size(Window window) const
    {
        unsigned int width, height;
        // Convert from core::XDo::Window to X11 window.
        const int ret_code = xdo_get_window_size(
            m_context, static_cast<::Window>(window),
            &width, &height);
        if (ret_code) {
            throw XDoError("xdo_get_window_size failed");
        }
        else {
            return std::make_pair(width, height);
        }
    }

}
