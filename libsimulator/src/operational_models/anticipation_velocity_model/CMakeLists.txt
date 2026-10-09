## anticipation_velocity_model sources
target_sources(simulator PRIVATE
    anticipation_velocity_model.cpp
    anticipation_velocity_model.hpp
    anticipation_velocity_model_state.hpp
)
target_include_directories(simulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})