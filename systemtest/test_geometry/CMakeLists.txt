list(APPEND clean_geometry_tests
    ${CMAKE_SOURCE_DIR}/systemtest/test_geometry/clean_geometry/inifile1.xml
    ${CMAKE_SOURCE_DIR}/systemtest/test_geometry/clean_geometry/inifile2.xml
    ${CMAKE_SOURCE_DIR}/systemtest/test_geometry/clean_geometry/inifile3.xml
    ${CMAKE_SOURCE_DIR}/systemtest/test_geometry/clean_geometry/inifile4.xml
    ${CMAKE_SOURCE_DIR}/systemtest/test_geometry/clean_geometry/inifile5.xml
    ${CMAKE_SOURCE_DIR}/systemtest/test_geometry/clean_geometry/inifile6.xml
    ${CMAKE_SOURCE_DIR}/systemtest/test_geometry/clean_geometry/inifile7.xml
    ${CMAKE_SOURCE_DIR}/systemtest/test_geometry/clean_geometry/inifile8.xml
    ${CMAKE_SOURCE_DIR}/systemtest/test_geometry/clean_geometry/inifile9-1.xml
    ${CMAKE_SOURCE_DIR}/systemtest/test_geometry/clean_geometry/inifile9-2.xml
    ${CMAKE_SOURCE_DIR}/systemtest/test_geometry/clean_geometry/inifile10.xml
    )
foreach(file ${clean_geometry_tests})
    get_filename_component(test ${file} NAME_WE)
    string(REGEX MATCH "[0-9]+[-0-9]*" test_number ${test})
    add_test(
        NAME correct_geometry-${test_number}
        COMMAND $<TARGET_FILE:jpscore> ${file}
        )
    set_tests_properties(correct_geometry-${test_number} PROPERTIES LABELS "CI:FAST")
endforeach()
