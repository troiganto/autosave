
#include <bandit/bandit.h>

using namespace bandit;

go_bandit([](){

    describe("our first test", [](){
        it("is a success", [&](){
            AssertThat(5, Equals(5));
        });
    });

});
