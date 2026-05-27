target_sources(simulator PRIVATE
    OperationalModel.hpp
    OperationalModelType.hpp
    OperationalModelUpdate.hpp
)
target_include_directories(simulator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/AnticipationVelocityModel
    ${CMAKE_CURRENT_SOURCE_DIR}/CollisionFreeSpeedModel
    ${CMAKE_CURRENT_SOURCE_DIR}/CollisionFreeSpeedModelV2
    ${CMAKE_CURRENT_SOURCE_DIR}/CollisionFreeSpeedModelV3
    ${CMAKE_CURRENT_SOURCE_DIR}/GeneralizedCentrifugalForceModel
    ${CMAKE_CURRENT_SOURCE_DIR}/SocialForceModel
    ${CMAKE_CURRENT_SOURCE_DIR}/WarpDriver
)

# include per-model lists
add_subdirectory(AnticipationVelocityModel)
add_subdirectory(CollisionFreeSpeedModel)
add_subdirectory(CollisionFreeSpeedModelV2)
add_subdirectory(CollisionFreeSpeedModelV3)
add_subdirectory(GeneralizedCentrifugalForceModel)
add_subdirectory(SocialForceModel)
add_subdirectory(WarpDriver)
