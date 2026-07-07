## SocialForceModel sources
target_sources(simulator PRIVATE
    SocialForceModel.cpp
    SocialForceModel.hpp
)
target_include_directories(simulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})