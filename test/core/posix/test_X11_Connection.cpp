
#include "core/KeyCombo.hpp"
#include "core/posix/X11/Connection.hpp"
#include "core/posix/POpenHelper.hpp"

#include <bandit/bandit.h>
#include <string>
#include <stdexcept>

// POSIX header
extern "C" {
    #include <unistd.h>
}

using namespace bandit;

go_bandit([](){

    using namespace core;

    describe("Connection", [](){

        describe("basically", [](){

            it("can be constructed", [](){
                X11::Connection x;
            });

            it("throws if xdo_new fails", [&](){
                AssertThrows(X11::Error, X11::Connection {":1337"});
            });

            it("throws when asked for the parent of an invalid window", [&](){
                X11::Connection x;
                AssertThrows(X11::Error, x.get_parent(0xdeadbeef));
                AssertThat(LastException<X11::Error>().get_error_code(),
                           Equals(3 /*XCB_WINDOW*/));
            });

            it("throws when asked to send an invalid key", [&](){
                X11::Connection x;
                constexpr unsigned int XK_VOID_SYMBOL = 0xffffff;
                AssertThrows(X11::Error, x.send_key_combo(KeyCombo(XK_VOID_SYMBOL)));
                AssertThat(LastException<X11::Error>().get_error_code(),
                           Equals(XK_VOID_SYMBOL));
            });

        });

        describe("active window", [](){

            it("gives a window ID", [&](){
                X11::Connection x;
                const auto window = x.get_active_window();
                const auto title = x.get_window_title(window);

                AssertThat(window, Is().GreaterThan(0));
                AssertThat(title, Is().Not().EqualTo(""));
            });

            it("gives a process ID", [&](){
                X11::Connection x;
                const auto window = x.get_active_window();
                const auto pid = x.get_pid_window(window);
                // 100 should exclude most system processes.
                AssertThat(pid, Is().GreaterThan(100));
            });

        });

        describe("input focus", [](){

            it("gives a window ID", [&](){
                X11::Connection x;
                const auto window = x.get_input_focus();
                AssertThat(window, Is().GreaterThan(0));
            });

            it("gives the active window or a descendant of it", [&](){
                X11::Connection x;
                const auto focus = x.get_input_focus();
                const auto active = x.get_active_window();
                AssertThat(x.is_descendant(focus, active), IsTrue());
            });

            it("can't give a window title", [&](){
                X11::Connection x;
                const auto window = x.get_input_focus();
                AssertThat(x.get_window_title(window), Equals(""));
                //~ AssertThrows(X11::Error, x.get_window_title(window));
            });

            it("can't give a process ID", [&](){
                X11::Connection x;
                const auto window = x.get_input_focus();
                AssertThrows(X11::Error, x.get_pid_window(window));
            });

        });

        xdescribe("interacts with gedit and", [&](){

            it("can get its PID", [&](){
                X11::Connection x;
                POpenHelper poh("gedit & sleep 2s && kill -1 %1");
                sleep(1);
                const auto window = x.get_active_window();
                const auto pid = x.get_pid_window(window);
                sleep(2);
                AssertThat(pid, Is().Not().EqualTo(0));
            });

            it("recognizes open and closed windows", [&](){
                X11::Connection x;
                POpenHelper poh("gedit & sleep 2s && kill -1 %1 2>/dev/null");
                sleep(1);
                const auto window = x.get_active_window();
                AssertThat(x.window_exists(window), IsTrue());
                sleep(2);
                AssertThat(x.window_exists(window), IsFalse());
            });

            it("can send key combos", [&](){
                // Open gedit and get its window.
                X11::Connection x;
                POpenHelper poh("gedit & sleep 5s && kill -1 %1");
                sleep(1);
                const auto window = x.get_active_window();
                AssertThat(x.window_exists(window), IsTrue());
                // Send some letters.
                for (auto letter : std::string("PRAISE THE SMOOZE")) {
                    x.send_key_combo(KeyCombo(letter));
                    usleep(100000);
                }
                // Alt+F4 --> quit
                sleep(1);
                x.send_key_combo(KeyCombo(true, false, false, 0xffc1));
                // Alt+n --> don't save
                sleep(1);
                x.send_key_combo(KeyCombo(true, false, false, 'n'));
                // Make sure the window is gone.
                sleep(1);
                AssertThat(x.window_exists(window), IsFalse());
            });
        });
    });

});
