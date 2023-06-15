function(print_var variable)
    message(STATUS "${variable}=${${variable}}")
endfunction()

# There is a long outstanding issue with CMake where CMake does not support
# relative paths in CMAKE_PREFIX_PATH but does not report anything. Many hours
# have been wasted over this behavior. We are checking to save your this to
# save everyone some headache. :)
function(check_prefix_path)
    if(CMAKE_PREFIX_PATH)
        foreach(path ${CMAKE_PREFIX_PATH})
            if(NOT IS_ABSOLUTE ${path})
                message(FATAL_ERROR "CMake does not support relative paths for CMAKE_PREFIX_PATH! [${path}]")
            endif()
        endforeach()
    endif()
endfunction()

function(find_python_library python_lib)
    if(WIN32)
        # Using python from vcpkg messes with cmakes find package.
        # find_package(Python) should prefer a venv python over a concrete one,
        # this no longer works when using the vcpkg toolchain file.
        # On windows we just "hope" to find the right python as there is currently 
        # no way to force use of the venv python
        set(PYTHON_EXECUTABLE "python")
    else()
        set(PYTHON_EXECUTABLE ${Python_EXECUTABLE})
    endif()
    
    execute_process(
            COMMAND
            ${PYTHON_EXECUTABLE} "-c" "import ${python_lib}; print(${python_lib}.__version__)"
            RESULT_VARIABLE status
            OUTPUT_VARIABLE version
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT status)
        print_var(python_lib)
        print_var(version)
    else()
        message(FATAL_ERROR "Did not find package ${python_lib}")
    endif()
endfunction()
