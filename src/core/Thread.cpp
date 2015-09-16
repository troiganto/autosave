/*
 * Thread.cpp
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


#include "core/Thread.hpp"

#include <stdexcept>

using namespace std::literals;

namespace core
{
    Thread::Thread(const Settings& settings)
        : m_settings(settings)
        , m_countdown(settings.get_interval())
        , m_thread()
        , m_req_state(RequestedState::RUNNING)
        , m_req_state_mutex()
        , m_req_state_cv()
    {

    }

    Thread::~Thread()
    {
        stop();
    }

    void Thread::set_requested_state(RequestedState new_state) noexcept
    {
        // Note: STOPPED is a dead end.
        if (m_req_state != RequestedState::STOPPED) {
            // Reading the required state is fine, but for setting it,
            // we need a lock on the mutex.
            {
                std::lock_guard lock(m_req_state_mutex);
                m_req_state = new_state;
            }
            // State changed, lock released. Notify the sending thread.
            m_req_state_cv.notify_one();
        }
    }

    void Thread::main()
    {
        // This guarantees m_req_state to be consistent as long as we're
        // not waiting.
        std::unique_lock lock(m_req_state_mutex);
        while (!should_stop()) {
            // Wait for 1 second or until the requested state changes.
            m_req_state_cv.wait_for(lock, 1s);
            if (may_act()) {
                // Do something.
            }
            else if (should_pause()) {
                // Wait until state "paused" is no longer requested.
                m_req_state.wait(lock, [](){ return !should_pause(); });
            }
        }
        // Cleanup.
    }
}
