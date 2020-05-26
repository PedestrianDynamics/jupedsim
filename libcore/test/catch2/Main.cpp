#define CATCH_CONFIG_RUNNER

#include <catch2/catch.hpp>
#include <libshared>

int main(int argc, char * argv[])
{
    Logging::Guard guard;

    Logging::SetLogLevel(Logging::Level::Off);

    int result = Catch::Session().run(argc, argv);

    return result;
}
