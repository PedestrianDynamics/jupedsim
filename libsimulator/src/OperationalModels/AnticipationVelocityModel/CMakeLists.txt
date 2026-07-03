## AnticipationVelocityModel sources
target_sources(simulator PRIVATE
    AnticipationVelocityModel.cpp
    AnticipationVelocityModel.hpp
    AnticipationVelocityModelBuilder.cpp
    AnticipationVelocityModelBuilder.hpp
)
target_include_directories(simulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})