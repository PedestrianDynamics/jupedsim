## CollisionFreeSpeedModel sources
target_sources(simulator PRIVATE
    CollisionFreeSpeedModelV3.cpp
    CollisionFreeSpeedModelV3.hpp
    CollisionFreeSpeedModelV3State.hpp
)
target_include_directories(simulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})