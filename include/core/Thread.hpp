/*
 * Thread.hpp
 * A background thread that regularly sends key events to other applications.
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

#include "core/Settings.hpp"
#include "core/Timer.hpp"
#include "core/threadsafe.hpp"

#include <chrono>
#include <thread>

/*!The namespace of the Autosave core library.
 *
 * This namespace contains all components necessary for the bare running
 * of Autosave.
 */
namespace core
{
    /*!A class that represents the key-event-sending background thread of Autosave.
     *
     * Its behavior is determined by a passed core::Settings object.
     * It may be paused or terminated from outside using a write-only
     * shared variable. (It is read-only inside the thread.)
     * It gives information about the status of its timer to the outside
     * through a read-only shared variable. (It is write-only inside the
     * thread.)
     */
    class Thread
    {
    public:
        //!Used by the main thread to control the sending thread.
        enum class RequestedState {
            RUNNING,    //!< Do actual work (counting down, sending, etc.).
            PAUSED,     //!< Do nothing until resume() is called.
            STOPPED     //!< End the thread as soon as possible.
        };

    public:
        /*!Create an instance.
         *
         * \arg settings contains all configurable options of the thread.
         *      Note that the settings are passed by value to ensure its
         *      lifetime within the thread.
         *
         * \sa start()
         */
        Thread(Settings settings);

        /*!Waits until the sending thread is stopped and destructs this
         * instance.
         *
         * \sa stop()
         */
        ~Thread();

        /*!Start the sending thread.
         *
         * If the thread status is `STOPPED`, start a new thread.
         * If the thread is already `RUNNING` or `PAUSED`, do nothing.
         *
         * By having a start() function separate from the constructor,
         * we can be sure that this instance is fully constructed when
         * the thread starts.
         *
         * \sa pause(), resume(), stop()
         */
        void start();

        /*!Pause the sending thread.
         *
         * If the thread status is `RUNNING`, make it wait until resume()
         * is called.
         * If the thread is already `PAUSED` or `STOPPED`, do nothing.
         *
         * \sa start(), resume(), stop()
         */
        inline void pause() noexcept {
            set_requested_state(RequestedState::PAUSED);
        }

        /*!Resume a paused sending thread.
         *
         * If the thread status is `PAUSED`, make it continue its work.
         * If the thread is already `RUNNING` or `STOPPED`, do nothing.
         *
         * \sa start(), pause(), stop()
         */
        inline void resume() noexcept {
            set_requested_state(RequestedState::RUNNING);
        }

        /*!Stop a running thread.
         *
         * Make the thread quit as soon as possible and wait until it
         * has finished.
         * If the thread is already `STOPPED`, do nothing.
         *
         * \sa start(), pause(), resume()
         */
        inline void stop() {
            set_requested_state(RequestedState::STOPPED);
            if (m_thread.joinable()) {
                m_thread.join();
            }
        }

        /*!Return the state of the sending timer.
         *
         * \returns the read-end of the pipe for the sending timer state.
         */
        inline threadsafe::Reader<Timer> get_timer() {
            return m_timer.reader();
        }

    private:
        const Settings m_settings;  //!< All options to the sending thread.
        std::thread m_thread;       //!< Representation of the sending thread.

        //! Pipe to the sending thread that sends pause and resume commands.
        threadsafe::Pipe<RequestedState> m_req_state;
        //! Pipe from the sending thread that delivers the timer state.
        threadsafe::Pipe<Timer> m_timer;

        /*! Internal implementation of setting the requested state.
         *
         *  \sa pause(), resume(), stop()
         */
        void set_requested_state(RequestedState new_state) noexcept;
    };
}
