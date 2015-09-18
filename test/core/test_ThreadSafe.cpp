
#include <bandit/bandit.h>
#include <iostream>
#include <utility>
#include <string>
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

    describe("The core::ThreadSafe", [](){

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

            it("copies lvalue arguments", [&](){
                Mock mock;
                core::ThreadSafe<Mock> ts(mock);
                check_mock(ts.get(), false, true, false, false, false);
            });

            it("moves rvalue arguments", [&](){
                core::ThreadSafe<Mock> ts {Mock()};
                check_mock(ts.get(), false, false, true, false, false);
            });

        });

        describe("setter", [&](){

            it("copy-assigns lvalues arguments", [&](){
                core::ThreadSafe<Mock> ts {Mock()};
                Mock mock;
                ts.set(mock);
                check_mock(ts.get(), false, false, true, true, false);
            });

            it("move-assigns rvalue arguments", [&](){
                Mock mock;
                core::ThreadSafe<Mock> ts(mock);
                ts.set(Mock());
                check_mock(ts.get(), false, true, false, false, true);
            });

            it("allows inter-thread communication", [&](){
                using namespace std::literals;
                core::ThreadSafe<int> ts(0);
                const core::ThreadSafe<int>& const_ts = ts;
                std::unique_lock<std::mutex> lock(ts.m_mutex);
                const_ts.m_cv.wait_for(lock, 1s);
            });

        });

    });

});
