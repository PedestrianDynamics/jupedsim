# Due to different random numbers depending on the plattform we are not able to do the reference tests on all platforms.
# This is just a workaround to have random tests at all.
# Fot platform independent reference test we have two possibilites:
#  1. Use platform independent random numbers.
#     Would be nice for reproducability.
#  2. Have platform depending expected results
if(UNIX AND NOT APPLE)
    list(APPEND reference_tests
        ${CMAKE_SOURCE_DIR}/systemtest/reference_tests/RT01_corridor_GCFM_global-shortest
        ${CMAKE_SOURCE_DIR}/systemtest/reference_tests/RT02_corridor_velocity_global-shortest
        )

    # For each directory in ${reference_tests} at least an inifile.xml and an output_expected directory is required.
    # The following script will run jpscore using the inifile.
    # After a successful run of jpscore every file in output_expected is compared to the file with the same name in the results folder.
    # The inifile must be configured to store the result files in the default output folder.

    foreach(test_dir ${reference_tests})
        get_filename_component(test_name ${test_dir} NAME)
        add_test(
            NAME reference_test-${test_name}
            COMMAND $<TARGET_FILE:jpscore> ${test_dir}/inifile.xml
            )

        file(GLOB output_files_expected ${test_dir}/output_expected/*)
        foreach(file_expected ${output_files_expected})
            get_filename_component(file_expected_name_we ${file_expected} NAME_WE)
            get_filename_component(file_expected_name ${file_expected} NAME)
            add_test(
                NAME reference_test-${test_name}-diff-${file_expected_name_we}
                COMMAND ${CMAKE_COMMAND} -E compare_files
                results/${file_expected_name}
                ${file_expected}
                )
            set_tests_properties(reference_test-${test_name}-diff-${file_expected_name_we} PROPERTIES DEPENDS reference_test-${test_name})
            set_tests_properties(reference_test-${test_name}-diff-${file_expected_name_we} PROPERTIES LABELS "CI:FAST")
        endforeach()
        set_tests_properties(reference_test-${test_name} PROPERTIES LABELS "CI:FAST")
    endforeach()
endif()
