## collision_free_speed_model_tordeux2016 sources
target_sources(simulator PRIVATE
    motivation_collision_free_speed_model.cpp
    motivation_collision_free_speed_model.hpp
    motivation_collision_free_speed_model_state.hpp
)
target_include_directories(simulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})