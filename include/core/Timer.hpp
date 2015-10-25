/*
 * Timer.hpp
 * A class that allows counting down and maintaining state about this.
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

#include <chrono>

namespace core
{
    /*!A class that allows counting down and maintaining state about this.
     *
     * A Timer has three attributes:
     * * The length, measured in seconds,
     * * the position, measured in seconds as well,
     * * the state, which takes on one of several values.
     *
     * If a Timer of a certain length is created, its position is initially
     * set to its length.
     * It may then ticked down until it reaches zero.
     * It can be reset at any time.
     */
    class Timer
    {
    public:
        //! Possible values of the Timer state.
        enum class State {
            WAITING,    //!< The default state.
            COUNTDOWN,  //!< The position is in the range 1..5.
            OVERTIME,   //!< The position is below or equal to 0.
            SUCCESSFUL  //!< Special state set by succeed().
        };
    public:

        /*!Create an instance.
         *
         * \arg length specifies the number of ticks it takes to reach zero.
         */
        explicit Timer(std::chrono::seconds length) noexcept
            : m_length(length)
            , m_position(length)
            , m_state(WAITING)
        {}

        //! \returns the length of the Timer.
        inline std::chrono::seconds length() const {
            return m_length;
        }

        //! \returns the current position of the Timer.
        inline std::chrono::seconds position() const {
            return m_position;
        }

        //! \returns the current state of the Timer.
        inline State state() const {
            return m_state;
        }

        /*!Reduce this Timer's position by 1 second and change state accordingly.
         *
         * \note
         * The position may become zero or even negative.
         * As soon as the position reaches zero, the state is changed
         * to State::OVERTIME.
         *
         * \note
         * If the current state is State::SUCCESSFUL, the position is
         * not changed and the state is changed to State::WAITING instead.
         */
        void tick() {
            if (m_state == State::SUCCESSFUL) {
                m_state = State::WAITING;
            }
            else {
                --m_position;
                switch (m_position) {
                    case 5:
                    m_state = State::COUNTDOWN;
                    break;
                    case 0:
                    m_state = State::OVERTIME;
                }
            }
        }

        /*!Set this Timer's position back to its length and its state
         * back to State::WAITING.
         */
        void reset() {
            m_position = m_length;
            m_state = State::WAITING;
        }

        /*!Set this Timer's position back to its length and its state
         * to State::SUCCESSFUL.
         *
         * This essentially gives the next period of counting down one
         * extra second, but also carries different semantics than
         * just resetting the timer.
         */
        void succeed() {
            m_position = m_length;
            m_state = State::SUCCESSFUL;
        }

    private:
        const std::chrono::seconds m_length;
        std::chrono::seconds m_position;
        State m_state;
    };
}
