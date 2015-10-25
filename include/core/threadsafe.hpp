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
    /*!Namespace of two classes which allow threads to communicate with
     * each other in a safe manner.
     */
    namespace threadsafe
    {
        // Forward-declaration.
        template<typename var_type>
        class Reader;

        /*!This class wraps a member attribute with a mutex and a
         * condition variable which allows to set this variable in a
         * thread-safe manner and to wait for changes to that variable.
         */
        template<typename var_type>
        class Pipe
        {
            std::mutex m_mutex;
            std::condition_variable m_cv;
            var_type m_value;

        public:
            /*!Create an instance.
             *
             * This creates a new pipe and initializes its `value`
             * attribute with \a value.
             *
             * This function throws if copying or moving \a value throws.
             */
            template<typename arg_type>
            explicit Pipe(arg_type&& value)
                noexcept(noexcept(var_type(std::forward<arg_type>(value))))
                : m_value(std::forward<arg_type>(value))
            {}

            //!Pipes cannot be copied.
            Pipe(const Pipe<var_type>& rhs) = delete;

            /*!Move an instance.
             *
             * This function throws if moving or copying Pipe::value() throws.
             */
            Pipe(Pipe<var_type>&& rhs) noexcept(noexcept(var_type(rhs.m_value)))
                : m_value(rhs.m_value)
            {}

            //!Pipes cannot be copied.
            Pipe& operator =(const Pipe<var_type>& rhs) = delete;

            /*!Move an instance.
             *
             * This function throws if moving or copying Pipe::value() throws.
             */
            Pipe& operator =(Pipe<var_type>&& rhs) noexcept(noexcept(var_type(rhs.m_value))) = default;

            //! \return A reference to the pipe's underlying `mutex`.
            inline operator std::mutex&() noexcept {
                return m_mutex;
            }

            /*!Convenience function serving as a short-hand constructor
             * of `std::unique_lock`.
             *
             * \return An `std::unique_lock` of this pipe's underlying mutex.
             */
            inline std::unique_lock<std::mutex> lock() noexcept {
                return std::unique_lock<std::mutex>(m_mutex);
            }

            //! \return A reference to the pipe's underlying `condition_variable`.
            inline std::condition_variable& cv() noexcept {
                return m_cv;
            }

            /*! \return A reference to the pipe's underlying `value`.
             *
             * \sa Pipe::signal()
             */
            inline var_type& value() noexcept {
                return m_value;
            }

            //! \overload
            inline const var_type& value() const noexcept {
                return m_value;
            }

            /*!Notify all threads waiting for Pipe::cv().
             */
            void signal() noexcept
            {
                m_cv.notify_all();
            }

            /*!Set Pipe::value() to a new \a value and notify all
             * threads waiting for Pipe::cv().
             *
             * This function throws if assigning \a value throws.
             */
            template<typename arg_type>
            void signal(arg_type&& value)
                noexcept(noexcept(m_value = std::forward<arg_type>(value)))
            {
                m_value = std::forward<arg_type>(value);
                m_cv.notify_all();
            }

            /*!Acquire a lock on this pipe's `mutex` before calling Pipe::signal().
             *
             * Note that the behavior is undefined if this thread already
             * has a lock on this pipe's `mutex`.
             *
             * This function throws if assigning \a value throws.
             */
            template<typename arg_type>
            void locked_signal(arg_type&& value)
                noexcept(noexcept(m_value = std::forward<arg_type>(value)))
            {
                {
                    auto temp_lock = lock();
                    m_value = std::forward<arg_type>(value);
                }
                m_cv.notify_all();
            }

            /*!Convenience function serving as an explicit conversion
             * function to ThreadSafe::Reader.
             *
             * \returns A ThreadSafe::Reader associated with this pipe.
             */
            inline Reader<var_type> reader() noexcept {
                return Reader<var_type>(*this);
            }
        };

        template<typename var_type>
        class Reader
        {
            Pipe<var_type>& m_parent;

        public:
            /*!Create an instance.
             *
             * This creates a new reader associated with the passed Pipe.
             */
            Reader(Pipe<var_type>& parent) noexcept
                : m_parent(parent)
            {}

            //! Copy an instance.
            Reader(const Reader<var_type>& rhs) noexcept = default;

            //! Move an instance.
            Reader(Reader<var_type>&& rhs) noexcept = default;

            //! Copy an instance.
            Reader& operator =(const Reader<var_type>& rhs) = default;

            //! Move an instance.
            Reader& operator =(Reader<var_type>&& rhs) noexcept = default;

            //! \sa Pipe::operator std::mutex&()
            inline operator std::mutex&() noexcept {
                return m_parent;
            }

            //! \sa Pipe::lock()
            inline std::unique_lock<std::mutex> lock() noexcept {
                return m_parent.lock();
            }

            //! \sa Pipe::cv()
            inline std::condition_variable& cv() noexcept {
                return m_parent.cv();
            }

            //! \sa Pipe::value()
            inline const var_type& value() const noexcept {
                return m_parent.value();
            }
        };
    }
}
