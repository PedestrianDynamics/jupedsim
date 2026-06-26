## CollisionFreeSpeedModel sources
target_sources(simulator PRIVATE
    CollisionFreeSpeedModel.cpp
    CollisionFreeSpeedModel.hpp
    CollisionFreeSpeedModelBuilder.cpp
    CollisionFreeSpeedModelBuilder.hpp
    CollisionFreeSpeedModelData.hpp
    CollisionFreeSpeedModelUpdate.hpp
)
target_include_directories(simulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})