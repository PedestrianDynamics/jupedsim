## AnticipationVelocityModel sources
target_sources(simulator PRIVATE
    AnticipationVelocityModel.cpp
    AnticipationVelocityModel.hpp
    AnticipationVelocityModelState.hpp
)
target_include_directories(simulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})