
#include <bandit/bandit.h>
#include "core/Process.hpp"

using namespace bandit;

go_bandit([](){

    using core::Process;

    describe("The Process class", [](){

        it("can get the calling process.", [&](){
            Process proc;
            AssertThat(proc.started_by("/bin/bash"), IsTrue());
        });

    });

});
