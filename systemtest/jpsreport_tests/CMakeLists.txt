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
add_test(NAME I01_test_reference_data COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_I/I01_test_reference_data/I01_test_reference_data.py ${jpsreport_exe})
add_test(NAME I02_corridor_low_density_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_I/I02_corridor_low_density_IFD/I02_corridor_low_density_IFD.py ${jpsreport_exe})
add_test(NAME I03_corridor_medium_density_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_I/I03_corridor_medium_density_IFD/I03_corridor_medium_density_IFD.py ${jpsreport_exe})
add_test(NAME I04_corridor_high_density_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_I/I04_corridor_high_density_IFD/I04_corridor_high_density_IFD.py ${jpsreport_exe})
add_test(NAME I05_corridor_low_cut_off COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_I/I05_corridor_low_cut_off/I05_corridor_low_cut_off.py ${jpsreport_exe})
add_test(NAME I06_corridor_high_cut_off COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_I/I06_corridor_high_cut_off/I06_corridor_high_cut_off.py ${jpsreport_exe})
add_test(NAME I07_corridor_geometry_intersection COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_I/I07_corridor_geometry_intersection/I07_corridor_geometry_intersection.py ${jpsreport_exe})
add_test(NAME I08_corridor_one_person COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_I/I08_corridor_one_person/I08_corridor_one_person.py ${jpsreport_exe})
add_test(NAME J01_test_reference_data COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_J/J01_test_reference_data/J01_test_reference_data.py ${jpsreport_exe})
add_test(NAME J02_corridor_low_density COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_J/J02_corridor_low_density/J02_corridor_low_density.py ${jpsreport_exe})
add_test(NAME J03_corridor_medium_density COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_J/J03_corridor_medium_density/J03_corridor_medium_density.py ${jpsreport_exe})
add_test(NAME J04_corridor_high_density COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_J/J04_corridor_high_density/J04_corridor_high_density.py ${jpsreport_exe})
add_test(NAME J05_corridor_low_density_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_J/J05_corridor_low_density_IFD/J05_corridor_low_density_IFD.py ${jpsreport_exe})
add_test(NAME J06_corridor_medium_density_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_J/J06_corridor_medium_density_IFD/J06_corridor_medium_density_IFD.py ${jpsreport_exe})
add_test(NAME J07_corridor_high_density_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_J/J07_corridor_high_density_IFD/J07_corridor_high_density_IFD.py ${jpsreport_exe})
add_test(NAME J08_corridor_low_cut_off COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_J/J08_corridor_low_cut_off/J08_corridor_low_cut_off.py ${jpsreport_exe})
add_test(NAME J09_corridor_high_cut_off COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_J/J09_corridor_high_cut_off/J09_corridor_high_cut_off.py ${jpsreport_exe})
add_test(NAME J10_corridor_cut_off_velocity COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_J/J10_corridor_cut_off_velocity/J10_corridor_cut_off_velocity.py ${jpsreport_exe})

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
  I01_test_reference_data
  I02_corridor_low_density_IFD
  I03_corridor_medium_density_IFD
  I04_corridor_high_density_IFD
  I05_corridor_low_cut_off
  I06_corridor_high_cut_off
  I07_corridor_geometry_intersection
  I08_corridor_one_person
  J01_test_reference_data
  J02_corridor_low_density
  J03_corridor_medium_density
  J04_corridor_high_density
  J05_corridor_low_density_IFD
  J06_corridor_medium_density_IFD
  J07_corridor_high_density_IFD
  J08_corridor_low_cut_off
  J09_corridor_high_cut_off
  J10_corridor_cut_off_velocity
  PROPERTIES LABELS "CI:FAST;jpsreport")

