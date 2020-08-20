#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

extern "C" {
#include <log.h>
};

int main(int argc, char **argv) {
    log_set_quiet(true);

    int result = Catch::Session().run(argc, argv);

    log_set_quiet(false);

    return result;
}
