/*
 * ThreadSafe.hpp
 * Wrapper class around variables that manages simple setting and getting
 * in a thread-safe manner.
 *
 * Its behavior is determined by a passed core::Settings object.
 * It may be paused or terminated from outside using a write-only shared
 * variable. (It is read-only inside the thread.)
 * It gives information to the outside through a read-only shared
 * variable. (It is write-only inside the thread.)
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

#include <utility>
#include <mutex>
#include <condition_variable>

namespace core
{
    namespace ThreadSafe
    {
        template<typename var_type>
        class Reader;

        template<typename var_type>
        class Pipe
        {
        public:
            // Template parameter `arg_type` is necessary to invoke the
            // rules necessary for perfect forwarding.

            template<typename arg_type>
            Pipe(arg_type&& value)
                noexcept(noexcept(var_type(std::forward<arg_type>(value))))
                : m_value(std::forward<arg_type>(value))
            {}
            Pipe(const Pipe<var_type>& rhs) = delete;
            Pipe(Pipe<var_type>&& rhs) = delete;
            Pipe& operator =(const Pipe<var_type>& rhs) = delete;
            Pipe& operator =(Pipe<var_type>&& rhs) = delete;

            template<typename arg_type>
            inline void set(arg_type&& value) {
                set_no_notify(std::forward<arg_type>(value));
                cv.notify_all();
            }

            template<typename arg_type>
            inline void set_no_notify(arg_type&& value) {
                using namespace std;
                unique_lock<recursive_mutex> lock(m_mutex);
                m_value = forward<arg_type>(value);
            }

            inline const var_type& get() const noexcept {
                return m_value;
            }

            inline std::unique_lock<std::recursive_mutex> get_lock() {
                using namespace std;
                return unique_lock<recursive_mutex>(m_mutex);
            }

            inline Reader<var_type> reader() noexcept {
                return Reader<var_type>(*this);
            }

        public:
            std::condition_variable_any cv;

        private:
            var_type m_value;
            std::recursive_mutex m_mutex;
        };

        template<typename var_type>
        class Reader
        {
        public:
            Reader(Pipe<var_type>& parent) noexcept
                : cv(parent.cv)
                , m_parent(parent)
            {}

            Reader(const Reader<var_type>& rhs) noexcept
                : cv(rhs.cv)
                , m_parent(rhs.m_parent)
            {}

            Reader(Reader<var_type>&& rhs) noexcept
                : cv(rhs.cv)
                , m_parent(rhs.m_parent)
            {}

            Reader& operator =(const Reader<var_type>& rhs) = delete;
            Reader& operator =(Reader<var_type>&& rhs) noexcept = delete;

            inline const var_type& get() const noexcept {
                return m_parent.get();
            }

            inline std::unique_lock<std::recursive_mutex> get_lock() {
                return m_parent.get_lock();
            }

        public:
            std::condition_variable_any& cv;

        private:
            Pipe<var_type>& m_parent;
        };
    }
}
