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
#include <iostream>

using namespace std::literals;

namespace core
{
    Thread::Thread(const Settings& settings)
        : m_settings(settings)
        , m_countdown(settings.get_interval())
        , m_thread()
        , m_req_state(RequestedState::STOPPED)
        , m_req_state_mutex()
        , m_req_state_cv()
    {
        std::cout << "Thread constructed" << std::endl;
    }

    Thread::~Thread()
    {
        stop();
        std::cout << "Thread destructed" << std::endl;
    }

    // By having a separate function for actually starting the thread,
    // we can be sure `this` is fully constructed when the thread
    // starts going.
    void Thread::start()
    {
        if (!m_thread.joinable() && m_req_state == RequestedState::STOPPED) {
            // Extra scope for automatic locking/unlocking.
            m_req_state = RequestedState::RUNNING;
            m_thread = std::thread(&Thread::main, this);
        }
    }

    void Thread::set_requested_state(RequestedState new_state) noexcept
    {
        if (m_req_state != RequestedState::STOPPED) {
            // Extra scope for automatic locking/unlocking.
            {
                std::lock_guard<std::mutex> lock(m_req_state_mutex);
                m_req_state = new_state;
            }
            m_req_state_cv.notify_one();
        }
    }

    void Thread::main() noexcept
    {
        std::unique_lock<std::mutex> lock(m_req_state_mutex);
        while (!should_stop()) {
            // Wait for 1 second or until the requested state changes.
            m_req_state_cv.wait_for(lock, 1s);
            if (may_act()) {
                step();
            }
            else if (should_pause()) {
                // Wait until state "paused" is no longer requested.
                m_req_state_cv.wait(lock, [this](){ return !should_pause(); });
            }
        }
        // Note: If *any* exception occurs here, the whole program is terminated.
    }

    void Thread::step()
    {
        //~ if delayed_sending_loop
            //~ if active window matches
                //~ send
            //~ else if no window matches
                //~ reset
            //~ else
                //~ pass
        //~ else if five seconds left
            //~ if no window matches
                //~ reset
            //~ else
                //~ start countdown
        //~ else if countdown at zero
            //~ if active window matches
                //~ send
            //~ else
                //~ go into delayed_sending_loop
        //~ else
            //~ pass
    }
}
