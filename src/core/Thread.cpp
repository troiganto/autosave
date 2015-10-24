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
#include <utility>

using namespace std::literals;

namespace core
{
    namespace
    {
        void thread_main( const Settings& settings
                        , threadsafe::Reader<Thread::RequestedState> req_state
                        , threadsafe::Pipe<Thread::TimerState>& timer_state
                        ) noexcept;
        void thread_step();
    }

    Thread::Thread(Settings settings)
        : m_settings(std::move(settings))
        , m_thread()
        , m_req_state(RequestedState::STOPPED)
        , m_timer_state(TimerState::WAITING)
    {}

    Thread::~Thread()
    {
        stop();
    }

    void Thread::start()
    {
        if (m_req_state.value() == RequestedState::STOPPED) {
            // If the thread hasn't quite stopped yet, wait for it.
            if (m_thread.joinable()) {
                m_thread.join();
            }
            m_req_state.value() = RequestedState::RUNNING;
            m_thread = std::thread( &thread_main
                                  , std::ref(m_settings)
                                  , m_req_state.reader()
                                  , std::ref(m_timer_state)
                                  );
        }
    }

    void Thread::set_requested_state(RequestedState new_state) noexcept
    {
        if (m_req_state.value() != RequestedState::STOPPED) {
            m_req_state.locked_signal(new_state);
        }
    }


    namespace
    {
        void thread_main( const Settings& settings
                        , threadsafe::Reader<Thread::RequestedState> req_state
                        , threadsafe::Pipe<Thread::TimerState>& timer_state
                        ) noexcept
        {
            auto req_state_lock = req_state.lock();
            while (req_state.value() != Thread::RequestedState::STOPPED) {
                // Wait for 1 second or until the requested state changes.
                req_state.cv().wait_for(req_state_lock, 1s);
                if (req_state.value() == Thread::RequestedState::RUNNING) {
                    thread_step();
                }
                else if (req_state.value() == Thread::RequestedState::PAUSED) {
                    // Wait until state "paused" is no longer requested.
                    req_state.cv().wait(req_state_lock, [&req_state](){
                        return req_state.value() != Thread::RequestedState::PAUSED;
                    });
                }
            }
            // Note: If *any* exception occurs here, the whole program is terminated.
        }

        void thread_step()
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
}
