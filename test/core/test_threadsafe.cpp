
#include "core/threadsafe.hpp"

#include <bandit/bandit.h>
#include <iostream>
#include <utility>
#include <string>
#include <thread>

using namespace bandit;

namespace
{
    struct Mock
    {
        Mock()
            : default_constructor_called(true)
            , copy_constructor_called(false)
            , move_constructor_called(false)
            , copy_assignment_called(false)
            , move_assignment_called(false)
        {}
        Mock(const Mock& rhs)
            : default_constructor_called(false)
            , copy_constructor_called(rhs.copy_constructor_called)
            , move_constructor_called(false)
            , copy_assignment_called(false)
            , move_assignment_called(false)
        {
            copy_constructor_called = true;
        }
        Mock(Mock&& rhs)
            : default_constructor_called(false)
            , copy_constructor_called(false)
            , move_constructor_called(true)
            , copy_assignment_called(false)
            , move_assignment_called(false)
        {
            rhs.default_constructor_called =
                rhs.copy_constructor_called =
                rhs.move_constructor_called =
                rhs.copy_assignment_called =
                rhs.move_assignment_called = false;
        }
        Mock& operator =(const Mock& rhs) {
            copy_assignment_called = rhs.copy_assignment_called;
            copy_assignment_called = true;
            return *this;
        }
        Mock& operator =(Mock&& rhs) {
            move_assignment_called = true;
            rhs.default_constructor_called =
                rhs.copy_constructor_called =
                rhs.move_constructor_called =
                rhs.copy_assignment_called =
                rhs.move_assignment_called = false;
            return *this;
        }

        bool default_constructor_called;
        bool copy_constructor_called;
        bool move_constructor_called;
        bool copy_assignment_called;
        bool move_assignment_called;
    };

}

go_bandit([](){

    describe("The Mock class", [](){

        using ::Mock;

        it("has a default constructor", [&](){
            Mock mock;
            AssertThat(mock.default_constructor_called, IsTrue());
        });
        it("has a copy constructor", [&](){
            Mock dummy;
            Mock mock(dummy);
            AssertThat(mock.copy_constructor_called, IsTrue());
        });
        it("has a move constructor", [&](){
            Mock dummy;
            Mock mock(std::move(dummy));
            AssertThat(mock.move_constructor_called, IsTrue());
        });
        it("has copy assignment", [&](){
            Mock dummy;
            Mock mock;
            mock = dummy;
            AssertThat(mock.default_constructor_called, IsTrue());
            AssertThat(mock.copy_constructor_called, IsFalse());
            AssertThat(mock.copy_assignment_called, IsTrue());
        });
        it("has move assignment", [&](){
            Mock mock = Mock();
            mock = Mock();
            AssertThat(mock.default_constructor_called, IsTrue());
            AssertThat(mock.copy_assignment_called, IsFalse());
            AssertThat(mock.move_assignment_called, IsTrue());
        });
    });

    describe("The core::ThreadSafe::Pipe", [](){

        auto check_mock = []( const Mock& mock, std::array<bool, 5> expected)
        {
            std::string where;
            try {
                where="Default Constructor";
                AssertThat(mock.default_constructor_called, Equals(expected[0]));
                where="Copy Constructor";
                AssertThat(mock.copy_constructor_called, Equals(expected[1]));
                where="Move Constructor";
                AssertThat(mock.move_constructor_called, Equals(expected[2]));
                where="Copy Assignment";
                AssertThat(mock.copy_assignment_called, Equals(expected[3]));
                where="Move Assignment";
                AssertThat(mock.move_assignment_called, Equals(expected[4]));
            }
            catch (const AssertionException& exc) {
                std::stringstream message;
                message << exc.GetMessage() << "Failed at check for "
                        << where << '\n';
                throw AssertionException( message.str()
                                        , exc.GetFilename()
                                        , exc.GetLineNumber()
                                        );
            }
        };

        describe("constructor", [&](){

            using core::threadsafe::Pipe;

            it("copies lvalue arguments", [&](){
                Mock mock;
                Pipe<Mock> pipe(mock);
                check_mock(pipe.value(), {false, true, false, false, false});
            });

            it("moves rvalue arguments", [&](){
                Pipe<Mock> pipe {Mock()};
                check_mock(pipe.value(), {false, false, true, false, false});
            });

            it("is noexcept for integers", [&](){
                AssertThat(noexcept(Pipe<int>(0)), IsTrue());
            });

            it("is noexcept when moving an std::string", [&](){
                std::string s;
                AssertThat(noexcept(Pipe<std::string>(std::move(s))), IsTrue());
            });

            it("may throw when copying an std::string", [&](){
                const std::string s;
                AssertThat(noexcept(Pipe<std::string>(s)), IsFalse());
            });

            it("is noexcept when moving a noexcept pipe", [&](){
                Pipe<int> pipe(0);
                AssertThat(noexcept(Pipe<int>(std::move(pipe))), IsTrue());
            });

            it("may throw when enforcing a string copy while moving a pipe", [&](){
                Pipe<const std::string> pipe("");
                AssertThat(noexcept(Pipe<const std::string>(std::move(pipe))), IsFalse());
            });

        });

        describe("member function signal", [&](){

            using core::threadsafe::Pipe;

            it("copy-assigns lvalues arguments", [&](){
                Pipe<Mock> pipe {Mock()};
                Mock mock;
                pipe.signal(mock);
                check_mock(pipe.value(), {false, false, true, true, false});
            });

            it("move-assigns rvalue arguments", [&](){
                Mock mock;
                Pipe<Mock> pipe(mock);
                pipe.signal(Mock());
                check_mock(pipe.value(), {false, true, false, false, true});
            });

            it("is noexcept for integers", [&](){
                Pipe<int> pipe(0);
                AssertThat(noexcept(pipe.signal(0)), IsTrue());
            });

            it("is noexcept iff move-assigning an std::string is noexcept", [&](){
                Pipe<std::string> pipe("");
                std::string s;
                AssertThat( noexcept(pipe.signal(std::move(s)))
                          , Equals(noexcept(s = std::string()))
                          );
            });

            it("may throw when copying an std::string", [&](){
                Pipe<std::string> pipe("");
                const std::string s;
                AssertThat(noexcept(pipe.signal(s)), IsFalse());
            });

            it("has the same noexcept spec as Pipe::locked_signal", [&](){
                Mock m;
                Pipe<int> a(0);
                Pipe<Mock> b(m);
                Pipe<std::string> c("");
                AssertThat(noexcept(a.signal(0)), Equals(noexcept(a.locked_signal(0))));
                AssertThat(noexcept(b.signal(m)), Equals(noexcept(b.locked_signal(m))));
                AssertThat(noexcept(c.signal("")), Equals(noexcept(c.locked_signal(""))));
            });

        });

        describe("class", [&](){

            using namespace core::threadsafe;
            using namespace std::literals;

            it("may be waited for", [&](){
                Pipe<int> pipe(0);
                auto lock = pipe.lock();
                pipe.cv().wait_for(lock, 100ms);
            });

            it("allows threads to receive information", [&](){
                Pipe<int> pipe(0);
                int actual = 0;
                int expected = 42;
                auto thread_func = [&actual](Reader<int> reader)
                {
                    // Wait until the reader gives a non-zero value.
                    auto lock = reader.lock();
                    reader.cv().wait(lock, [&](){ return reader.value() != 0; });
                    // Then set output to that value.
                    actual = reader.value();
                };
                // Start the thread, hand it a pipe reader.
                std::thread thread(thread_func, pipe.reader());
                // Wait until the thread is waiting.
                std::this_thread::sleep_for(100ms);
                // Now acquire the guaranteed-to-be free lock and wake the thread.
                pipe.locked_signal(expected);
                thread.join();
                AssertThat(actual, Equals(expected));
            });

            it("allows threads to send information", [&](){
                Pipe<int> pipe(0);
                auto thread_func = [](Pipe<int>& pipe)
                {
                    // Before signaling, wait until the parent thread
                    // releases its own lock.
                    pipe.locked_signal(42);
                };
                // Create reader, lock the pipe before the thread starts.
                auto reader = pipe.reader();
                auto lock = reader.lock();
                // Start the thread.
                std::thread thread(thread_func, std::ref(pipe));
                std::this_thread::sleep_for(100ms);
                AssertThat(thread.joinable(), IsTrue());
                // *Now* the thread may continue.
                reader.cv().wait(lock, [&](){ return reader.value() != 0; });
                thread.join();
                AssertThat(reader.value(), Equals(42));
            });

            it("may be used two at once", [&](){
                using namespace std;
                auto send_first = [](Pipe<bool>& send, Reader<bool> listen)
                {
                    auto lock = send.lock();
                    send.signal(true);
                    lock = listen.lock();
                    listen.cv().wait(lock, [&](){ return listen.value(); });
                };
                auto listen_first = [](Pipe<bool>& send, Reader<bool> listen)
                {
                    auto lock = listen.lock();
                    listen.cv().wait(lock, [&](){ return listen.value(); });
                    lock = send.lock();
                    send.signal(true);
                };
                Pipe<bool> back(false), forth(false);
                thread one(send_first, ref(back), forth.reader());
                thread two(listen_first, ref(forth), back.reader());
                one.join();
                two.join();
                AssertThat(back.value(), Is().True().And().EqualTo(forth.value()));
            });

            it("allow bi-directional communication", [&](){
                using namespace std;
                Pipe<int> data_pipe(0);
                Pipe<bool> ready_pipe(false);
                int output = 0;
                auto generate_data = [](Pipe<int>& data, Pipe<bool>& ready)
                {
                    for (int i=1; i<=10; ++i) {
                        auto lock = data.lock();
                        data.signal(i);
                        // Flag readiness and wait until the ready flag is cleared.
                        lock = ready.lock();
                        ready.signal(true);
                        ready.cv().wait(lock, [&](){ return !ready.value(); });
                    }
                    // Signal end of data.
                    data.signal(-1);
                    ready.signal(true);
                };
                auto process_data = [&output](Reader<int> data, Pipe<bool>& ready)
                {
                    int sum = 0;
                    int cur_val = 0;
                    do {
                        // Wait until the ready flag is set.
                        auto lock = ready.lock();
                        ready.cv().wait(lock, [&](){ return ready.value(); });
                        // Handle data. Generator is waiting till we clear ready.
                        cur_val = data.value();
                        if (cur_val >= 0) {
                            sum += data.value();
                        }
                        // Clear flag to get the next batch of data.
                        ready.signal(false);
                    } while (cur_val >= 0);
                    // Write final result to where the main thread can read it.
                    output = sum;
                };
                thread generator(generate_data, ref(data_pipe), ref(ready_pipe));
                thread processor(process_data, data_pipe.reader(), ref(ready_pipe));
                generator.join();
                processor.join();
                AssertThat(output, Equals(55));
                AssertThat(data_pipe.value(), Equals(-1));
                AssertThat(ready_pipe.value(), IsFalse());
            });

        });

    });

});
