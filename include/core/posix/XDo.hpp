/*
 * XDo.hpp
 *
 * Incomplete C++ wrapper around libxdo, mainly for automatic memory
 * management.
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

#include <string>
#include <stdexcept>
#include <utility>

extern "C" {
    // Forward declaration of otherwise hidden struct.
    struct xdo;
}

namespace core
{
    class XDoError : public std::runtime_error
    {
    public:
        explicit XDoError(const std::string& what) : std::runtime_error(what) {}
        explicit XDoError(const char* what) : std::runtime_error(what) {}
    };

    class XDo
    {
        public:
            XDo(const char* display=nullptr);
            ~XDo();

            // We avoid exposing the libX11 type Window because then,
            // we'd have to include X11/X.h. And this file pollutes
            // the global namespace *badly*.
            // X11/X.h defines Window as XID.
            // It further defines XID as either unsigned long or unsigned int.
            // By choosing unsigned long for core::XDo::Window, we play
            // it safe.
            typedef unsigned long Window;

            Window get_active_window() const;
            int get_pid_window(Window window) const;

            std::pair<unsigned int, unsigned int>
            get_window_size(Window window) const;

        private:
            struct xdo* m_context;
    };
}

