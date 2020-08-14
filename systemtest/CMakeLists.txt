# get used jpscore executable
set(jpscore_exe $<TARGET_FILE:jpscore>)
set(jpsreport_exe $<TARGET_FILE:jpsreport>)

add_subdirectory(test_geometry)
add_subdirectory(juelich_tests)
add_subdirectory(rimea_tests)
add_subdirectory(router_tests)
add_subdirectory(source_tests)
add_subdirectory(door_tests)
add_subdirectory(train_tests)
add_subdirectory(waiting_area_tests)
add_subdirectory(jpsreport_tests)
add_subdirectory(reference_tests)
