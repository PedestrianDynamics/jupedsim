## collision_free_speed_model_tordeux2016 sources
target_sources(simulator PRIVATE
    collision_free_speed_model_tordeux2016.cpp
    collision_free_speed_model_tordeux2016.hpp
    collision_free_speed_model_tordeux2016_state.hpp
)
target_include_directories(simulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})