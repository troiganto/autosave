/*
 * Thread.hpp
 * A background thread that regularly sends key events to other applications.
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

#include "core/Settings.hpp"

#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

/*!The namespace of the Autosave core library.
 *
 * This namespace contains all components necessary for the bare running
 * of Autosave.
 */
namespace core
{
    class Thread
    {
    public:
        // The RequestedState is used by the main thread to control
        // the sending thread.
        enum class RequestedState {
            RUNNING,    // Do actual work (counting down, sending, etc.).
            PAUSED,     // Do nothing until resume() is called.
            STOPPED     // End the thread as soon as possible.
        };

        // The SendState is used by the sending thread to give feedback
        // to the main thread.
        enum class SendState {
            WAITING,    // There is quite some time till the next send event.
            COUNTDOWN,  // Very close to the next send event.
            OVERTIME,   // Should send, but no target window is focused.
            SUCCESSFUL  // Successfully sent a keyboard event.
        };

    public:
        // Initializes the object and *then* starts the background thread.
        Thread(const Settings& settings);
        // Orders the background thread to stop and joins it.
        ~Thread();

        // Setter functions for the requested state.

        // stopped -> running
        void start();
        // running/paused -> paused
        inline void pause() noexcept {
            set_requested_state(RequestedState::PAUSED);
        }
        // running/paused -> running
        inline void resume() noexcept {
            set_requested_state(RequestedState::RUNNING);
        }
        // running/paused -> stopped, also join the thread.
        inline void stop() {
            set_requested_state(RequestedState::STOPPED);
            if (m_thread.joinable()) {
                m_thread.join();
            }
        }

        // Getter function for the sending thread's send state.


    private:
        const Settings& m_settings;
        std::chrono::seconds m_countdown;
        // The object representing the sending thread.
        std::thread m_thread;

        volatile RequestedState m_req_state;
        std::mutex m_req_state_mutex;
        std::condition_variable m_req_state_cv;
        // The setter is only used by the main thread.
        void set_requested_state(RequestedState new_state) noexcept;
        // The getters are only used by the sending thread.
        inline bool should_stop() const noexcept {
            return m_req_state == RequestedState::STOPPED;
        }
        inline bool may_act() const noexcept {
            return m_req_state == RequestedState::RUNNING;
        }
        inline bool should_pause() const noexcept {
            return m_req_state == RequestedState::PAUSED;
        }

        void set_send_state() noexcept;

        // The main routine of the thread.
        void main() noexcept;
        // The actions taken every second.
        void step();

    };
}
