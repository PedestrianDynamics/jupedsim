# Due to different random numbers depending on the platforms we are not able to do the reference tests on all platforms.
# This is just a workaround to have random tests at all.
# For platform independent reference test we have two possibilites:
#  1. Use platform independent random numbers.
#     Would be nice for reproducibility.
#  2. Have platform depending expected results
if (UNIX)
    file(GLOB reference_test_dir LIST_DIRECTORIES true "${CMAKE_SOURCE_DIR}/systemtest/reference_tests/RT*")
    print_var(reference_test_dir)

    foreach (test_dir ${reference_test_dir})
        get_filename_component(test_name ${test_dir} NAME)
        file(GLOB test_py "${test_dir}/runtest_reference_*.py")
        add_test(
                NAME ${test_name}
                COMMAND ${PYTHON_EXECUTABLE} ${test_py} ${jpscore_exe}
        )
        set_tests_properties(
                ${test_name}
                PROPERTIES LABELS "CI:FAST"
        )
    endforeach ()
endif ()
