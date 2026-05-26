## CollisionFreeSpeedModel sources
list(APPEND SIMULATOR_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/CollisionFreeSpeedModel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CollisionFreeSpeedModel.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CollisionFreeSpeedModelBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CollisionFreeSpeedModelBuilder.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CollisionFreeSpeedModelData.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CollisionFreeSpeedModelUpdate.hpp
)

list(APPEND SIMULATOR_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR})

set(SIMULATOR_SOURCES ${SIMULATOR_SOURCES} PARENT_SCOPE)
set(SIMULATOR_INCLUDE ${SIMULATOR_INCLUDE} PARENT_SCOPE)