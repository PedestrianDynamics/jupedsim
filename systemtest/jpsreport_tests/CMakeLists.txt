add_test(NAME 01_method_A COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_A/test01/01_method_A.py ${jpsreport_exe})
add_test(NAME 02_method_A COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_A/test02/02_method_A.py ${jpsreport_exe})
add_test(NAME 03_method_A COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_A/test03/03_method_A.py ${jpsreport_exe})
add_test(NAME 04_method_A COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_A/test04/04_method_A.py ${jpsreport_exe})
add_test(NAME 05_method_A COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_A/test05/05_method_A.py ${jpsreport_exe})
add_test(NAME 01_method_B COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_B/test01/01_method_B.py ${jpsreport_exe})
add_test(NAME 01_method_C COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_C/test01/01_method_C.py ${jpsreport_exe})
add_test(NAME D01_test_reference_data COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D01_test_reference_data/D01_test_reference_data.py ${jpsreport_exe})
add_test(NAME D02_corridor_low_density COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D02_corridor_low_density/D02_corridor_low_density.py ${jpsreport_exe})
add_test(NAME D03_corridor_medium_density COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D03_corridor_medium_density/D03_corridor_medium_density.py ${jpsreport_exe})
add_test(NAME D04_corridor_high_density COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D04_corridor_high_density/D04_corridor_high_density.py ${jpsreport_exe})
add_test(NAME D05_corridor_low_density_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D05_corridor_low_density_IFD/D05_corridor_low_density_IFD.py ${jpsreport_exe})
add_test(NAME D06_corridor_medium_density_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D06_corridor_medium_density_IFD/D06_corridor_medium_density_IFD.py ${jpsreport_exe})
add_test(NAME D07_corridor_high_density_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D07_corridor_high_density_IFD/D07_corridor_high_density_IFD.py ${jpsreport_exe})
add_test(NAME D08_corridor_low_cut_off COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D08_corridor_low_cut_off/D08_corridor_low_cut_off.py ${jpsreport_exe})
add_test(NAME D09_corridor_high_cut_off COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D09_corridor_high_cut_off/D09_corridor_high_cut_off.py ${jpsreport_exe})
add_test(NAME D10_corridor_geometry_intersection COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D10_corridor_geometry_intersection/D10_corridor_geometry_intersection.py ${jpsreport_exe})
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
  D01_test_reference_data
  D02_corridor_low_density
  D03_corridor_medium_density
  D04_corridor_high_density
  D05_corridor_low_density_IFD
  D06_corridor_medium_density_IFD
  D07_corridor_high_density_IFD
  D08_corridor_low_cut_off
  D09_corridor_high_cut_off
  D10_corridor_geometry_intersection
  01_method_I
  01_method_J
  PROPERTIES LABELS "CI:FAST;jpsreport")

