add_test(NAME 01_method_A COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_A/test01/01_method_A.py ${jpsreport_exe})
add_test(NAME 02_method_A COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_A/test02/02_method_A.py ${jpsreport_exe})
add_test(NAME 03_method_A COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_A/test03/03_method_A.py ${jpsreport_exe})
add_test(NAME 04_method_A COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_A/test04/04_method_A.py ${jpsreport_exe})
add_test(NAME 05_method_A COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_A/test05/05_method_A.py ${jpsreport_exe})
add_test(NAME 01_method_B COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_B/test01/01_method_B.py ${jpsreport_exe})
add_test(NAME 01_method_C COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_C/test01/01_method_C.py ${jpsreport_exe})
add_test(NAME D01_test_reference_data_no_blind_points COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D01_test_reference_data_no_blind_points/D01_test_reference_data_no_blind_points.py ${jpsreport_exe})
add_test(NAME D02_corridor_low_density_no_blind_points COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D02_corridor_low_density_no_blind_points/D02_corridor_low_density_no_blind_points.py ${jpsreport_exe})
add_test(NAME D03_corridor_medium_density_no_blind_points COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D03_corridor_medium_density_no_blind_points/D03_corridor_medium_density_no_blind_points.py ${jpsreport_exe})
add_test(NAME D04_corridor_high_density_no_blind_points COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D04_corridor_high_density_no_blind_points/D04_corridor_high_density_no_blind_points.py ${jpsreport_exe})
add_test(NAME D05_corridor_low_density_IFD_no_blind_points COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D05_corridor_low_density_IFD_no_blind_points/D05_corridor_low_density_IFD_no_blind_points.py ${jpsreport_exe})
add_test(NAME D06_corridor_medium_density_IFD_no_blind_points COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D06_corridor_medium_density_IFD_no_blind_points/D06_corridor_medium_density_IFD_no_blind_points.py ${jpsreport_exe})
add_test(NAME D07_corridor_high_density_IFD_no_blind_points COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D07_corridor_high_density_IFD_no_blind_points/D07_corridor_high_density_IFD_no_blind_points.py ${jpsreport_exe})
add_test(NAME D08_corridor_low_cut_off_no_blind_points COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D08_corridor_low_cut_off_no_blind_points/D08_corridor_low_cut_off_no_blind_points.py ${jpsreport_exe})
add_test(NAME D09_corridor_high_cut_off_no_blind_points COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D09_corridor_high_cut_off_no_blind_points/D09_corridor_high_cut_off_no_blind_points.py ${jpsreport_exe})
add_test(NAME D10_corridor_geometry_intersection_no_blind_points COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D10_corridor_geometry_intersection_no_blind_points/D10_corridor_geometry_intersection_no_blind_points.py ${jpsreport_exe})
add_test(NAME D11_test_reference_data_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D11_test_reference_data_global_IFD/D11_test_reference_data_global_IFD.py ${jpsreport_exe})
add_test(NAME D12_corridor_low_density_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D12_corridor_low_density_global_IFD/D12_corridor_low_density_global_IFD.py ${jpsreport_exe})
add_test(NAME D13_corridor_medium_density_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D13_corridor_medium_density_global_IFD/D13_corridor_medium_density_global_IFD.py ${jpsreport_exe})
add_test(NAME D14_corridor_high_density_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D14_corridor_high_density_global_IFD/D14_corridor_high_density_global_IFD.py ${jpsreport_exe})
add_test(NAME D15_corridor_low_cut_off_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D15_corridor_low_cut_off_global_IFD/D15_corridor_low_cut_off_global_IFD.py ${jpsreport_exe})
add_test(NAME D16_corridor_high_cut_off_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D16_corridor_high_cut_off_global_IFD/D16_corridor_high_cut_off_global_IFD.py ${jpsreport_exe})
add_test(NAME D17_corridor_geometry_intersection_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D17_corridor_geometry_intersection_global_IFD/D17_corridor_geometry_intersection_global_IFD.py ${jpsreport_exe})
add_test(NAME D18_corridor_one_person_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_D/D18_corridor_one_person_global_IFD/D18_corridor_one_person_global_IFD.py ${jpsreport_exe})
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

add_test(NAME reference_no_blind_points_cut_off_disabled COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/reference_tests/blind_points_disabled/cut_off_disabled/test_cut_off_disabled.py ${jpsreport_exe})
add_test(NAME reference_no_blind_points_cut_off_edges COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/reference_tests/blind_points_disabled/cut_off_edges/test_cut_off_edges.py ${jpsreport_exe})
add_test(NAME reference_no_blind_points_cut_off_enabled COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/reference_tests/blind_points_disabled/cut_off_enabled/test_cut_off_enabled.py ${jpsreport_exe})
add_test(NAME reference_no_blind_points_cut_off_large COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/reference_tests/blind_points_disabled/cut_off_large/test_cut_off_large.py ${jpsreport_exe})
add_test(NAME reference_no_blind_points_cut_off_small COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/reference_tests/blind_points_disabled/cut_off_small/test_cut_off_small.py ${jpsreport_exe})
add_test(NAME reference_no_blind_points_profiles COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/reference_tests/blind_points_disabled/profiles/test_profiles.py ${jpsreport_exe})

add_test(NAME reference_cut_off_disabled_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/reference_tests/global_IFD/cut_off_disabled/test_cut_off_disabled.py ${jpsreport_exe})
add_test(NAME reference_cut_off_edges_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/reference_tests/global_IFD/cut_off_edges/test_cut_off_edges.py ${jpsreport_exe})
add_test(NAME reference_cut_off_enabled_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/reference_tests/global_IFD/cut_off_enabled/test_cut_off_enabled.py ${jpsreport_exe})
add_test(NAME reference_cut_off_large_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/reference_tests/global_IFD/cut_off_large/test_cut_off_large.py ${jpsreport_exe})
add_test(NAME reference_cut_off_small_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/reference_tests/global_IFD/cut_off_small/test_cut_off_small.py ${jpsreport_exe})
add_test(NAME reference_subrooms_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/reference_tests/global_IFD/subrooms/test_subrooms.py ${jpsreport_exe})

add_test(NAME reference_methods_ABC COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/reference_tests/methods_ABC/test_methods_ABC.py ${jpsreport_exe})

set_tests_properties(
  01_method_A
  02_method_A
  03_method_A
  04_method_A
  05_method_A
  01_method_B
  01_method_C
  D01_test_reference_data_no_blind_points
  D02_corridor_low_density_no_blind_points
  D03_corridor_medium_density_no_blind_points
  D04_corridor_high_density_no_blind_points
  D05_corridor_low_density_IFD_no_blind_points
  D06_corridor_medium_density_IFD_no_blind_points
  D07_corridor_high_density_IFD_no_blind_points
  D08_corridor_low_cut_off_no_blind_points
  D09_corridor_high_cut_off_no_blind_points
  D10_corridor_geometry_intersection_no_blind_points
  D11_test_reference_data_global_IFD
  D12_corridor_low_density_global_IFD
  D13_corridor_medium_density_global_IFD
  D14_corridor_high_density_global_IFD
  D15_corridor_low_cut_off_global_IFD
  D16_corridor_high_cut_off_global_IFD
  D17_corridor_geometry_intersection_global_IFD
  D18_corridor_one_person_global_IFD
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
  reference_no_blind_points_cut_off_disabled
  reference_no_blind_points_cut_off_edges
  reference_no_blind_points_cut_off_enabled
  reference_no_blind_points_cut_off_large
  reference_no_blind_points_cut_off_small
  reference_no_blind_points_cut_off_disabled
  reference_no_blind_points_profiles
  reference_cut_off_edges_global_IFD
  reference_cut_off_enabled_global_IFD
  reference_cut_off_large_global_IFD
  reference_cut_off_small_global_IFD
  reference_subrooms_global_IFD
  reference_methods_ABC

  PROPERTIES LABELS "CI:FAST;jpsreport")

