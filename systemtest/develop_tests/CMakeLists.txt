add_test(NAME develop_doors_test-1 COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/systemtest/develop_tests/doors/runtest_dev_1.py)
set_tests_properties(
    develop_doors_test-1
    PROPERTIES LABELS "CI:MEDIUM")
