add_test(NAME 01_method_A COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_A/test01/01_method_A.py ${jpsreport_exe})
add_test(NAME 02_method_A COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_A/test02/02_method_A.py ${jpsreport_exe})
add_test(NAME 03_method_A COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_A/test03/03_method_A.py ${jpsreport_exe})
add_test(NAME 04_method_A COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_A/test04/04_method_A.py ${jpsreport_exe})
add_test(NAME 05_method_A COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_A/test05/05_method_A.py ${jpsreport_exe})
add_test(NAME 01_method_B COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_B/test01/01_method_B.py ${jpsreport_exe})
add_test(NAME 01_method_C COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_C/test01/01_method_C.py ${jpsreport_exe})
add_test(NAME 01_method_D COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/test01/01_method_D.py ${jpsreport_exe})
add_test(NAME 01_method_I COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_I/test01/01_method_I.py ${jpsreport_exe})
add_test(NAME 01_method_J COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_J/test01/01_method_J.py ${jpsreport_exe})

set_tests_properties(
  01_method_A
  02_method_A
  03_method_A
  04_method_A
  05_method_A
  01_method_B
  01_method_C
  01_method_D
  01_method_I
  01_method_J
  PROPERTIES LABELS "CI:FAST;jpsreport")

