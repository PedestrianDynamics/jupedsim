#define CATCH_CONFIG_RUNNER
#include "general/Logger.h"

#include <catch2/catch.hpp>

int main(int argc, char * argv[])
{
    Logging::Guard guard;
    Logging::SetLogLevel(Logging::Level::Off);

    int result = Catch::Session().run(argc, argv);

    return result;
}
