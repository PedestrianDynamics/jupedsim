add_test(NAME develop_doors_test-1 COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/systemtest/develop_tests/doors/runtest_dev_1.py)
add_test(NAME develop_sources_test-2 COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/systemtest/develop_tests/sources/runtest_dev_2.py)
set_tests_properties(
    develop_doors_test-1
    develop_sources_test-2
    PROPERTIES LABELS "CI:MEDIUM;DEV")
set_tests_properties(
    develop_sources_test-2
    PROPERTIES  WILL_FAIL TRUE LABELS "CI:FAST;DEV")


