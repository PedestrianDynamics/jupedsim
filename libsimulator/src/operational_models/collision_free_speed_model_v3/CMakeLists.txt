## collision_free_speed_model sources
target_sources(simulator PRIVATE
    collision_free_speed_model_v3.cpp
    collision_free_speed_model_v3.hpp
    collision_free_speed_model_v3_state.hpp
)
target_include_directories(simulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})