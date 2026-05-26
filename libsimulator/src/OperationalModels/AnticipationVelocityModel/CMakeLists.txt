## AnticipationVelocityModel sources
list(APPEND SIMULATOR_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/AnticipationVelocityModel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AnticipationVelocityModel.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AnticipationVelocityModelBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AnticipationVelocityModelBuilder.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AnticipationVelocityModelData.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AnticipationVelocityModelUpdate.hpp
)

list(APPEND SIMULATOR_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR})

set(SIMULATOR_SOURCES ${SIMULATOR_SOURCES} PARENT_SCOPE)
set(SIMULATOR_INCLUDE ${SIMULATOR_INCLUDE} PARENT_SCOPE)