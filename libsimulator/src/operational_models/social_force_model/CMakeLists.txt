## social_force_model sources
target_sources(simulator PRIVATE
    social_force_model.cpp
    social_force_model.hpp
    social_force_model_state.hpp
)
target_include_directories(simulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})