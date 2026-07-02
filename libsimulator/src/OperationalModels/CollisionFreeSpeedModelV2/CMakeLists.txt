## CollisionFreeSpeedModelV2 sources
target_sources(simulator PRIVATE
    CollisionFreeSpeedModelV2.cpp
    CollisionFreeSpeedModelV2.hpp
    CollisionFreeSpeedModelV2Builder.cpp
    CollisionFreeSpeedModelV2Builder.hpp
    CollisionFreeSpeedModelV2Data.hpp
)
target_include_directories(simulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})