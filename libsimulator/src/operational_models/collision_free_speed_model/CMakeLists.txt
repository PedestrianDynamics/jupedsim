## collision_free_speed_model sources
target_sources(simulator PRIVATE
    collision_free_speed_model.cpp
    collision_free_speed_model.hpp
    collision_free_speed_model_state.hpp
)
target_include_directories(simulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})