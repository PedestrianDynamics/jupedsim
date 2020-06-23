#include <gtest/gtest.h>

extern "C" {
#include <jps.h>
}

TEST(LibJPSTest, Init)
{
    finalize(nullptr);
}
