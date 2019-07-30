#include "IO/OutputHandler.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE JPSCORE_UNIT_TESTS
#include <boost/test/unit_test.hpp>

OutputHandler handler{};
OutputHandler* Log = &handler;
