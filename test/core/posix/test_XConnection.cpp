
#include <core/posix/XConnection.hpp>
#include <core/posix/POpenHelper.hpp>
using namespace core;


#include <bandit/bandit.h>

#include <string>
#include <stdexcept>

// POSIX header
extern "C" {
    #include <unistd.h>
}

using namespace bandit;

go_bandit([](){

    describe("The XConnection class", [](){

        it("can construct an instance of itself", [](){
            X11::XConnection x;
        });

        it("throws if xdo_new fails", [&](){
            auto create_with_invalid_argument = [](){
                X11::XConnection x(":1337");
                };
            AssertThrows(core::X11::Error, create_with_invalid_argument());
        });

        it("can give the active window", [&](){
            X11::XConnection x;
            const auto window = x.get_active_window();
            const auto title = x.get_window_title(window);
            AssertThat(window, Is().GreaterThan(1));
            AssertThat(title, Equals(""));
        });

        it("can get a window's PID", [&](){
            X11::XConnection x;
            POpenHelper poh("gedit & sleep 2s && kill -1 %1");
            sleep(1);
            const auto window = x.get_active_window();
            const auto pid = x.get_pid_window(window);
            sleep(2);
            AssertThat(pid, Is().Not().EqualTo(0));
        });

        it("recognizes open and closed windows", [&](){
            X11::XConnection x;
            POpenHelper poh("gedit & sleep 1s && kill -1 %1");
            usleep(500000);
            const auto window = x.get_active_window();
            AssertThat(x.window_exists(window), IsTrue());
            sleep(1);
            AssertThat(x.window_exists(window), IsFalse());
        });

    });

});
