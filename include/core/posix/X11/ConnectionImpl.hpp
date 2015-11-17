/*
 * ConnectionImpl.hpp
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

#include <core/KeyCombo.hpp>
#include <core/posix/X11/Connection.hpp>

extern "C" {
    #include <X11/keysym.h> // All key symbols as macros.
}
#include <xcb/xcb.h>
#include <xcb/xtest.h>
#include <xcb/xcb_keysyms.h>

#include <string>
#include <vector>
#include <memory>

namespace core
{
    // Forward declaration of KeyCombo class.
    class KeyCombo;

    /*! Namespace of X11-related classes and types.
     */
    namespace X11
    {
        //! \brief Implementation of XConnection.
        struct Connection::Impl
        {
        public:
            Impl(const char* display=nullptr);
            ~Impl();
            xcb_window_t get_parent(xcb_window_t child) const;
            bool is_descendant(xcb_window_t parent, xcb_window_t child) const;
            xcb_window_t get_input_focus() const noexcept;
            xcb_window_t get_active_window() const;
            unsigned long get_pid_window(xcb_window_t window) const;
            void send_key_combo(const KeyCombo& combo, xcb_window_t window);
            std::string get_window_title(xcb_window_t window) const;
            bool window_exists(xcb_window_t window) const;

        protected:
            std::vector<xcb_atom_t> intern_atoms(const std::vector<std::string>& names) const;
            std::unique_ptr<xcb_get_property_reply_t>
            get_property( xcb_window_t window
                        , xcb_atom_t prop
                        , xcb_atom_t prop_type
                        , uint32_t prop_len
                        ) const;
            xcb_keycode_t get_key_code(xcb_keysym_t symbol) const;
            void send_fake_input( xcb_window_t window
                                , uint8_t type
                                , xcb_keycode_t key_code
                                );
            std::vector<xcb_window_t> get_root_windows() const noexcept;
            xcb_window_t get_active_window_by_root(xcb_window_t root) const;
            xcb_window_t get_any_ancestor( std::vector<xcb_window_t> candidates
                                         , xcb_window_t child
                                         ) const;

        private:
            xcb_connection_t* m_c;

            xcb_atom_t m_pid_atom;
            xcb_atom_t m_win_name_atom;
            xcb_atom_t m_active_win_atom;

            xcb_key_symbols_t* m_syms;
            xcb_keycode_t m_alt_code;
            xcb_keycode_t m_ctrl_code;
            xcb_keycode_t m_shift_code;
        };
    }
}
