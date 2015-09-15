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
        , m_req_state(RequestedState::RUNNING)
        , m_thread()
    {

    }

    Thread::~Thread()
    {
        stop_and_join();
    }

    void Thread::pause() noexcept
    {
        if (m_req_state != RequestedState::STOPPED) {
            m_req_state = RequestedState::PAUSED;
        }
    }

    void Thread::resume() noexcept
    {
        if (m_req_state != RequestedState::STOPPED) {
            m_req_state = RequestedState::RUNNING;
        }
    }

    void Thread::stop_and_join()
    {
        // Signal thread to stop.
        m_req_state = RequestedState::STOPPED;
        // Wait only if there is a point to it.
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }

    void Thread::stop_and_detach()
    {
        // Signal thread to stop.
        m_req_state = RequestedState::STOPPED;
        // Wait only if there is a point to it.
        if (m_thread.joinable()) {
            m_thread.detach();
        }
    }

    void Thread::main()
    {
        try {
            while (!should_stop()) {
                if (may_act()) {
                    // Do something
                }
                // FIXME: Use condition variables instead.
                std::this_thread::sleep_for(1s);
            }
        }
        catch {
            // Maybe do some logging?
            // Catch all exceptions in any way to avoid std::terminate.
        }
        // Cleanup.
    }
}
