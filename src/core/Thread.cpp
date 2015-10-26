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
    namespace bgthread
    {
        static void main( const Settings& settings
                        , threadsafe::Reader<Thread::RequestedState> req_state
                        , threadsafe::Pipe<Timer>& timer
                        ) noexcept;
        static void step( const Settings& settings
                        , threadsafe::Pipe<Timer>& pipe
                        );
    }

    Thread::Thread(Settings settings)
        : m_settings(std::move(settings))
        , m_thread()
        , m_req_state(RequestedState::STOPPED)
        , m_timer(m_settings.interval())
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
            m_thread = std::thread( &bgthread::main
                                  , std::ref(m_settings)
                                  , m_req_state.reader()
                                  , std::ref(m_timer)
                                  );
        }
    }

    void Thread::set_requested_state(RequestedState new_state) noexcept
    {
        if (m_req_state.value() != RequestedState::STOPPED) {
            m_req_state.locked_signal(new_state);
        }
    }


    void bgthread::main( const Settings& settings
                       , threadsafe::Reader<Thread::RequestedState> req_state
                       , threadsafe::Pipe<Timer>& timer
                       ) noexcept
    {
        auto should_continue = [&req_state](){
            return req_state.value() != Thread::RequestedState::PAUSED;
        };
        auto req_state_lock = req_state.lock();
        // Main loop.
        while (req_state.value() != Thread::RequestedState::STOPPED) {
            req_state.cv().wait_for(req_state_lock, 1s);
            switch (req_state.value()) {
                case Thread::RequestedState::PAUSED:
                    req_state.cv().wait(req_state_lock, should_continue);
                    break;
                case Thread::RequestedState::RUNNING:
                    step(settings, timer);
                    break;
                default: break;
            }
        }
        // Note: If *any* exception occurs here, the whole program is terminated.
    }

    void bgthread::step( const Settings& settings
                       , threadsafe::Pipe<Timer>& pipe
                       )
    {
        auto lock = pipe.lock();
        Timer& timer = pipe.value();
        bool should_signal = false;

        timer.tick();
        switch (timer.state()) {
            case Timer::State::OVERTIME:
                if (false/*active_window_matches()*/) {
                    /*send();*/
                    timer.succeed();
                    should_signal = true;
                }
                else if (timer.position() == Timer::overtime_pos()) {
                    // This branch is activated periodically due to
                    // Timer::underflow_pos().
                    if (false/*no_window_matches()*/) {
                        timer.reset();
                    }
                    should_signal = true;
                }
                break;
            case Timer::State::COUNTDOWN:
                if (timer.position() == Timer::countdown_pos() /*&&
                    no_window_matches()*/)
                {
                    timer.reset();
                }
                // In countdown state, we must always signal.
                // That's the point of counting down.
                should_signal = true;
                break;
            // case Timer::State::WAITING:
            default:
                if (timer.position() == timer.length()) {
                    // We have just returned from
                    // Timer::State::SUCCESSFUL, signal this.
                    should_signal = true;
                }
        }
        if (should_signal) {
            lock.release();
            pipe.signal();
        }
    }
}
