target_sources(simulator PRIVATE
    CustomModel.hpp
    CustomModelState.hpp
    FormatAny.hpp
)
target_include_directories(simulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
