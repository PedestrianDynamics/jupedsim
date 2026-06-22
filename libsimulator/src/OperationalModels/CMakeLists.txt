target_sources(simulator PRIVATE
    OperationalModel.hpp
    OperationalModelType.hpp
    OperationalModelUpdate.hpp
)
target_include_directories(simulator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# include per-model lists
add_subdirectory(AnticipationVelocityModel)
add_subdirectory(CollisionFreeSpeedModel)
add_subdirectory(CollisionFreeSpeedModelV2)
add_subdirectory(CollisionFreeSpeedModelV3)
add_subdirectory(GeneralizedCentrifugalForceModel)
add_subdirectory(CustomModel)
add_subdirectory(SocialForceModel)
add_subdirectory(WarpDriver)
