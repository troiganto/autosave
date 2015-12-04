
#include "core/Timer.hpp"

#include <bandit/bandit.h>
#include <iostream>
#include <stdexcept>
#include <chrono>

using namespace bandit;

go_bandit([](){

    describe("Timers", [](){

        using namespace std::literals;
        using core::Timer;

        it("throw when constructed with an invalid length", [&](){
            AssertThrows(std::out_of_range, Timer {3s});
            AssertThrows(std::out_of_range, Timer {Timer::countdown_pos()});
            AssertThrows(std::out_of_range, Timer {-5s});
        });

        it("go into countdown 3 seconds before 0.", [&](){
            Timer timer {5s};
            timer.tick();
            Assert::That(timer.state(), Equals(Timer::State::WAITING));
            timer.tick();
            Assert::That(timer.state(), Equals(Timer::State::COUNTDOWN));
        });

        it("go into overtime at 0.", [&](){
            Timer timer {6s};
            for (int i=0; i<5; ++i) {
                timer.tick();
            }
            Assert::That(timer.state(), Equals(Timer::State::COUNTDOWN));
            timer.tick();
            Assert::That(timer.state(), Equals(Timer::State::OVERTIME));

        });

        it("reset to zero after five seconds of overtime.", [&](){
            Timer timer {6s};
            for (int i=0; i<10; ++i) {
                timer.tick();
            }
            Assert::That(timer.state(), Equals(Timer::State::OVERTIME));
            Assert::That(timer.position(), Equals(-4s));
            timer.tick();
            Assert::That(timer.state(), Equals(Timer::State::OVERTIME));
            Assert::That(timer.position(), Equals(Timer::overtime_pos()));
        });

        it("have an additional tick after succeed().", [&](){
            Timer timer {6s};
            for (int i=0; i<6; ++i) {
                timer.tick();
            }
            timer.succeed();
            Assert::That(timer.state(), Equals(Timer::State::SUCCESSFUL));
            Assert::That(timer.position(), Equals(timer.length()));
            timer.tick();
            Assert::That(timer.state(), Equals(Timer::State::WAITING));
            Assert::That(timer.position(), Equals(timer.length()));
        });

        it("have don't have an additional tick after reset().", [&](){
            Timer timer {7s};
            for (int i=0; i<7; ++i) {
                timer.tick();
            }
            timer.reset();
            Assert::That(timer.state(), Equals(Timer::State::WAITING));
            Assert::That(timer.position(), Equals(timer.length()));
            timer.tick();
            Assert::That(timer.state(), Equals(Timer::State::WAITING));
            Assert::That(timer.position(), Equals(6s));
        });

    });

});
