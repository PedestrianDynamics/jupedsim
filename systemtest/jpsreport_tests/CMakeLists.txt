message(STATUS "report: ${CMAKE_CURRENT_SOURCE_DIR}")

add_test(NAME jpsreport_method_A COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_A/runtest_method_A.py)
set_tests_properties(
    jpsreport_method_A
    PROPERTIES LABELS "CI:FAST;jpsreport")


