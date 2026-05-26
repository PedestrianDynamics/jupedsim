# include per-model lists
add_subdirectory(AnticipationVelocityModel)
add_subdirectory(CollisionFreeSpeedModel)
add_subdirectory(CollisionFreeSpeedModelV2)
add_subdirectory(CollisionFreeSpeedModelV3)
add_subdirectory(GeneralizedCentrifugalForceModel)
add_subdirectory(SocialForceModel)
add_subdirectory(WarpDriver)

## OperationalModels central list
list(APPEND SIMULATOR_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/OperationalDecisionSystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OperationalModel.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OperationalModelUpdate.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OperationalModelType.hpp
)

list(APPEND SIMULATOR_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR})

set(SIMULATOR_SOURCES ${SIMULATOR_SOURCES} PARENT_SCOPE)
set(SIMULATOR_INCLUDE ${SIMULATOR_INCLUDE} PARENT_SCOPE)