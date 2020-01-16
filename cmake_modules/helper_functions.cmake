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