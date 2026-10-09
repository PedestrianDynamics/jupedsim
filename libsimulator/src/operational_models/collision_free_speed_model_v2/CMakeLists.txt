## collision_free_speed_model_v2 sources
target_sources(simulator PRIVATE
    collision_free_speed_model_v2.cpp
    collision_free_speed_model_v2.hpp
    collision_free_speed_model_v2_state.hpp
)
target_include_directories(simulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})