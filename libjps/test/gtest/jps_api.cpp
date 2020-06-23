#include <gtest/gtest.h>

extern "C" {
#include <jps.h>
}

TEST(LibJPSTest, Init)
{
    JPS_finalize(nullptr);
}
