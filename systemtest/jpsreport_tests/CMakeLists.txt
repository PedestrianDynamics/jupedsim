add_test(NAME 01_method_A COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_A/01_method_A.py)
add_test(NAME 01_method_B COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_B/01_method_B.py)
add_test(NAME 01_method_C COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_C/01_method_C.py)
add_test(NAME 01_method_D COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/01_method_D.py)
set_tests_properties(
    01_method_A
    01_method_B
    01_method_C
    01_method_D
    PROPERTIES LABELS "CI:FAST;jpsreport")


