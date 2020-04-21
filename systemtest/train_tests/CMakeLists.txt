add_test(NAME train_test_simple COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/systemtest/train_tests/simple_train_test/runtest_simple_train.py ${jpscore_exe})

set_tests_properties(
        train_test_simple
        PROPERTIES LABELS "CI:FAST")
