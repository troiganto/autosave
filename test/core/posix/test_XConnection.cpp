
#include <core/KeyCombo.hpp>
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

            AssertThat(window, Is().GreaterThan(0));
            AssertThat(title, Is().Not().EqualTo(""));
        });

        it("can give the input focus", [&](){
            X11::XConnection x;
            const auto window = x.get_input_focus();
            AssertThat(window, Is().GreaterThan(0));
        });

        it("knows the difference between input focus and active window", [&](){
            X11::XConnection x;
            const auto focus = x.get_input_focus();
            const auto active = x.get_active_window();
            AssertThat(x.get_parent(focus), Equals(active));
        });

        it("throws an exception when asked to send an invalid key", [&](){
            X11::XConnection x;
            constexpr unsigned int XK_VOID_SYMBOL = 0xffffff;
            AssertThrows(X11::Error, [&](){
                x.send_key_combo(KeyCombo(XK_VOID_SYMBOL));
            }());
            AssertThat(LastException<X11::Error>().get_error_code(),
                       Equals(XK_VOID_SYMBOL))
        });

        describe("when interacting with gedit", [&](){

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
                POpenHelper poh("gedit & sleep 2s && kill -1 %1 2>/dev/null");
                sleep(1);
                const auto window = x.get_active_window();
                AssertThat(x.window_exists(window), IsTrue());
                sleep(2);
                AssertThat(x.window_exists(window), IsFalse());
            });

            it("can send key combos", [&](){
                // Open gedit and get its window.
                X11::XConnection x;
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
