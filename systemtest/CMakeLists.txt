# get used jpscore executable
set(jpscore_exe $<TARGET_FILE:jpscore>)

add_subdirectory(juelich_tests)
add_subdirectory(router_tests)
add_subdirectory(door_tests)
