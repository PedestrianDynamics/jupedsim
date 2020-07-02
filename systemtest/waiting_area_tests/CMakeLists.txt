add_test(NAME waiting_area_routing_room COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/systemtest/waiting_area_tests/routing_room_test/runtest_routing_room.py ${jpscore_exe})
set_tests_properties(
        waiting_area_routing_room
        PROPERTIES LABELS "CI:FAST")

add_test(NAME waiting_area_routing_subroom COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/systemtest/waiting_area_tests/routing_subroom_test/runtest_routing_subroom.py ${jpscore_exe})
set_tests_properties(
        waiting_area_routing_subroom
        PROPERTIES LABELS "CI:FAST")
