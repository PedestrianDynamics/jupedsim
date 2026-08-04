## Generalized Centrifugal Force Model sources
target_sources(simulator PRIVATE
    WarpDriverModel.cpp
    WarpDriverModel.hpp
    WarpDriverModelState.hpp
)
target_include_directories(simulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})