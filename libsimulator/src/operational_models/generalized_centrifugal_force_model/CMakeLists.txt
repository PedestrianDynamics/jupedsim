## Generalized Centrifugal Force Model sources
target_sources(simulator PRIVATE
    generalized_centrifugal_force_model.cpp
    generalized_centrifugal_force_model.hpp
    generalized_centrifugal_force_model_state.hpp
)
target_include_directories(simulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})