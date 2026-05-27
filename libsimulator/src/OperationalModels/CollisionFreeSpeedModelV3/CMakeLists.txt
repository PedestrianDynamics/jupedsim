## CollisionFreeSpeedModel sources
target_sources(simulator PRIVATE
    CollisionFreeSpeedModelV3.cpp
    CollisionFreeSpeedModelV3.hpp
    CollisionFreeSpeedModelV3Builder.cpp
    CollisionFreeSpeedModelV3Builder.hpp
    CollisionFreeSpeedModelV3Data.hpp
    CollisionFreeSpeedModelV3Update.hpp
)
target_include_directories(simulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})