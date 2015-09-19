
#include <bandit/bandit.h>
#include <iostream>
#include <utility>
#include <string>
#include <thread>
#include <system_error>
#include "core/ThreadSafe.hpp"

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

        auto check_mock = []( const Mock& mock, bool exp_defc, bool exp_copc
                            , bool exp_movc, bool exp_copa, bool exp_mova
                            )
        {
            std::string where;
            try {
                where="Default Constructor";
                AssertThat(mock.default_constructor_called, Equals(exp_defc));
                where="Copy Constructor";
                AssertThat(mock.copy_constructor_called, Equals(exp_copc));
                where="Move Constructor";
                AssertThat(mock.move_constructor_called, Equals(exp_movc));
                where="Copy Assignment";
                AssertThat(mock.copy_assignment_called, Equals(exp_copa));
                where="Move Assignment";
                AssertThat(mock.move_assignment_called, Equals(exp_mova));
            }
            catch (const AssertionException& exc) {
                std::string message = exc.GetMessage();
                message += "Failed at check for ";
                message += where;
                message.push_back('\n');
                throw AssertionException( message
                                        , exc.GetFilename()
                                        , exc.GetLineNumber()
                                        );
            }
        };

        describe("constructor", [&](){

            using core::ThreadSafe::Pipe;

            it("copies lvalue arguments", [&](){
                Mock mock;
                Pipe<Mock> pipe(mock);
                check_mock(pipe.get(), false, true, false, false, false);
            });

            it("moves rvalue arguments", [&](){
                Pipe<Mock> pipe {Mock()};
                check_mock(pipe.get(), false, false, true, false, false);
            });

        });

        describe("setter", [&](){

            using core::ThreadSafe::Pipe;

            it("copy-assigns lvalues arguments", [&](){
                Pipe<Mock> pipe {Mock()};
                Mock mock;
                pipe.set(mock);
                check_mock(pipe.get(), false, false, true, true, false);
            });

            it("move-assigns rvalue arguments", [&](){
                Mock mock;
                Pipe<Mock> pipe(mock);
                pipe.set(Mock());
                check_mock(pipe.get(), false, true, false, false, true);
            });

        });

        describe("class", [&](){

            using namespace core::ThreadSafe;
            using namespace std::literals;

            it("may be waited for", [&](){
                Pipe<int> pipe(0);
                auto lock = pipe.get_lock();
                pipe.cv.wait_for(lock, 100ms);
            });

            it("allows threads to receive information", [&](){
                auto thread_func = [](Reader<int> reader, int& output)
                {
                    // Wait until the reader gives a non-zero value.
                    auto lock = reader.get_lock();
                    reader.cv.wait(lock, [&](){ return reader.get() != 0; });
                    // Then set output to that value.
                    output = reader.get();
                };
                // Create a pipe to communicate with the thread.
                Pipe<int> pipe(0);
                int actual = 0;
                int expected = 42;
                // Start the thread, hand it a pipe reader.
                std::thread thread(thread_func, pipe.reader(), std::ref(actual));
                std::this_thread::sleep_for(100ms);
                pipe.set(expected);
                thread.join();
                AssertThat(actual, Equals(expected));
            });

            it("allows threads to send information", [&](){
                auto thread_func = [](Pipe<int>& pipe)
                {
                    pipe.set(42);
                };
                Pipe<int> pipe(0);
                // Create reader, lock the pipe till we're waiting
                auto reader = pipe.reader();
                auto lock = reader.get_lock();
                // Start the thread.
                std::thread thread(thread_func, std::ref(pipe));
                std::this_thread::sleep_for(100ms);
                AssertThat(thread.joinable(), IsTrue());
                // *Now* the thread may continue.
                reader.cv.wait(lock, [&](){ return reader.get() != 0; });
                thread.join();
                AssertThat(reader.get(), Equals(42));
            });

            it("allows threads to communicate with each other", [&](){
                using namespace std;
                auto generate_data = [](Pipe<int>& data, Reader<bool> ready)
                {
                    for (int i=1; i<=10; ++i) {
                        data.set(i);
                        auto lock = ready.get_lock();
                        ready.cv.wait(lock, [&](){ return ready.get(); });
                    }
                };
                auto process_data = [](Reader<int> data, Pipe<bool>& ready, int& output)
                {
                    int sum = 0;
                    auto lock = data.get_lock();
                    int cur_val = 0;
                    while (true) {
                        ready.set(true);
                        bool timeout = !data.cv.wait_for(
                            lock, 100ms, [&](){ return data.get() != cur_val; });
                        ready.set(false);
                        if (timeout) {
                            break;
                        }
                        cur_val = data.get();
                        sum += cur_val;
                    }
                    output = sum;
                };
                Pipe<int> data_pipe(0);
                Pipe<bool> ready_pipe(false);
                int output = 0;
                thread generator(generate_data, ref(data_pipe), ready_pipe.reader());
                thread processor(process_data, data_pipe.reader(), ref(ready_pipe), ref(output));
                generator.join();
                processor.join();
                AssertThat(output, Equals(55));
            });

        });

    });

});
