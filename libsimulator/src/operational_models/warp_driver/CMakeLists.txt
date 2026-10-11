## Generalized Centrifugal Force Model sources
target_sources(simulator PRIVATE
    warp_driver_model.cpp
    warp_driver_model.hpp
    warp_driver_model_state.hpp
)
target_include_directories(simulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})