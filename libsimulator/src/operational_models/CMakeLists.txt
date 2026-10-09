target_sources(simulator PRIVATE
    operational_model.hpp
    operational_model_state.hpp
    operational_model_type.hpp
)
target_include_directories(simulator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# include per-model lists
add_subdirectory(anticipation_velocity_model)
add_subdirectory(collision_free_speed_model)
add_subdirectory(collision_free_speed_model_v2)
add_subdirectory(collision_free_speed_model_v3)
add_subdirectory(custom_model)
add_subdirectory(generalized_centrifugal_force_model)
add_subdirectory(social_force_model)
add_subdirectory(warp_driver)
