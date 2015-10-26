
#include "core/posix/POpenHelper.hpp"

#include <bandit/bandit.h>
#include <string>
#include <stdexcept>

// POSIX header
extern "C" {
    #include <unistd.h>
}

using namespace core;
using namespace bandit;

go_bandit([](){

    describe("A POpenHelper", [](){

        it("has a default constructor", [](){
            POpenHelper poh;
        });

        it("gives exit code 127 if a command does not exist", [&](){
            POpenHelper poh("does_not_exist 2>/dev/null");
            AssertThat(poh.close(), Equals(127));
        });

        it("gives exit code 1 there is a syntax error", [&](){
            POpenHelper poh("echo 'missing single quote");
            AssertThat(poh.close(), Equals(1));
        });

        it("returns the error code of the program", [&](){
            POpenHelper poh("true");
            AssertThat(poh.close(), Equals(0));
            poh.open("false");
            AssertThat(poh.close(), Equals(1));
        });

        it("can return the stdout of a program.", [&](){
            POpenHelper poh("echo This is a test.");
            AssertThat(poh.get_output(), Equals("This is a test.\n"));
        });

    });

});
