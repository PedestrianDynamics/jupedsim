file(GLOB_RECURSE test_py_files "${CMAKE_SOURCE_DIR}/systemtest/source_tests/*/*_source_*.py")
foreach (file ${test_py_files})
    get_filename_component(test ${file} NAME_WE)
    add_test(
        NAME ${test}
        COMMAND ${PYTHON_EXECUTABLE} ${file}
        )
    set_tests_properties(
            ${test}
            PROPERTIES LABELS "CI:FAST")
endforeach ()