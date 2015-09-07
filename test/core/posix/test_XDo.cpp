
#include <core/posix/XDo.hpp>
using namespace core;


#include <bandit/bandit.h>

#include <string>
#include <stdexcept>

using namespace bandit;

go_bandit([](){

    describe("The XDo class", [](){

        it("can construct an instance of itself", [](){
            XDo xdo;
        });

        it("throws if xdo_new fails", [&](){
            auto create_with_invalid_argument = [](){
                XDo xdo(":1337");
                };
            AssertThrows(core::XDoError, create_with_invalid_argument());
        });

        it("can give the active window", [&](){
            XDo xdo;
            const auto size = xdo.get_active_window_size();
            AssertThat(size.first, Is().GreaterThan(0));
            AssertThat(size.second, Is().GreaterThan(0));
        });

        it("can get a window's PID", [&](){
            XDo xdo;
            const int pid = xdo.get_active_window_pid();
            AssertThat(pid, Is().Not().EqualTo(0));
        });

    });

});
