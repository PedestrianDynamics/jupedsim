add_test(NAME juelich_test-1 COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/systemtest/juelich_tests/test_1/runtest_juelich_1.py ${jpscore_exe})
add_test(NAME juelich_test-2 COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/systemtest/juelich_tests/test_2/runtest_juelich_2.py ${jpscore_exe})
add_test(NAME juelich_test-3 COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/systemtest/juelich_tests/test_3/runtest_juelich_3.py ${jpscore_exe})
add_test(NAME juelich_test-4 COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/systemtest/juelich_tests/test_4/runtest_juelich_4.py ${jpscore_exe})
add_test(NAME juelich_test-5 COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/systemtest/juelich_tests/test_5/runtest_juelich_5.py ${jpscore_exe})
add_test(NAME juelich_test-6 COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/systemtest/juelich_tests/test_6/runtest_juelich_6.py ${jpscore_exe})
add_test(NAME juelich_test-9 COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/systemtest/juelich_tests/test_9/runtest_juelich_9.py ${jpscore_exe})
add_test(NAME juelich_test-11 COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/systemtest/juelich_tests/test_11/runtest_juelich_11.py ${jpscore_exe})
add_test(NAME juelich_test-12 COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/systemtest/juelich_tests/test_12/runtest_juelich_12.py ${jpscore_exe})
set_tests_properties(
        juelich_test-1
        juelich_test-2
        juelich_test-3
        juelich_test-4
        juelich_test-5
        juelich_test-6
        juelich_test-9
        juelich_test-11
        juelich_test-12
        PROPERTIES LABELS "CI:FAST")


add_test(NAME juelich_test-8 COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/systemtest/juelich_tests/test_8/runtest_juelich_8.py ${jpscore_exe})
add_test(NAME juelich_test-14 COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/systemtest/juelich_tests/test_14/runtest_juelich_14.py ${jpscore_exe})
