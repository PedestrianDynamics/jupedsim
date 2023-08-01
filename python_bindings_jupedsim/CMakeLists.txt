add_library(py_jupedsim MODULE
    bindings_jupedsim.cpp
)

target_link_libraries(py_jupedsim
    PRIVATE
        pybind11::module
        pybind11::lto
        pybind11::windows_extras
        fmt::fmt
        jupedsim_obj
        common
)

pybind11_extension(py_jupedsim)

if(NOT MSVC AND NOT ${CMAKE_BUILD_TYPE} MATCHES Debug|RelWithDebInfo)
    # Strip unnecessary sections of the binary on Linux/macOS
    pybind11_strip(py_jupedsim)
endif()

set_target_properties(py_jupedsim 
    PROPERTIES 
        CXX_VISIBILITY_PRESET "hidden"
        CUDA_VISIBILITY_PRESET "hidden"
)

