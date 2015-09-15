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

namespace core
{
    class Thread
    {
    public:
        // Initializes the object and *then* starts the background thread.
        Thread(const Settings& settings);
        // Orders the background thread to stop and joins it.
        ~Thread();

        // The thread keeps running, but it stops sending events.
        // Do nothing if the thread has previously been requested to stop.
        void pause() noexcept;
        // Resume sending if previously paused.
        // Do nothing if the thread has previously been requested to stop.
        void resume() noexcept;
        // Stop running as soon as possible *and* join.
        void stop_and_join();
        // Same as `stop`, but returns immediately by detaching the thread.
        void stop_and_detach();

    private:
        // Enum used to control running and pausing of the thread.
        enum class RequestedState {
            RUNNING,
            PAUSED,
            STOPPED
        };

        const Settings& m_settings;
        std::chrono::seconds m_countdown;

        // First shared variable.
        RequestedState m_req_state;
        std::thread m_thread;

        // The main routine of the thread.
        void main() noexcept;

        // If this returns true, the thread should exit ASAP.
        constexpr bool should_stop() const noexcept {
            return m_req_state == RequestedState::STOPPED;
        }
        // If this returns true, the thread may count down and send key events.
        constexpr bool may_act() const noexcept {
            return m_req_state == RequestedState::RUNNING;
        }
    };
}
