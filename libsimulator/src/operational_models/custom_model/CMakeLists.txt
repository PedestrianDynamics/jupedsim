target_sources(simulator PRIVATE
    custom_model.hpp
    custom_model_state.hpp
    format_any.hpp
)
target_include_directories(simulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
