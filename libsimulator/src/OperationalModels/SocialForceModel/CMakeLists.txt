## SocialForceModel sources
target_sources(simulator PRIVATE
    SocialForceModel.cpp
    SocialForceModel.hpp
    SocialForceModelBuilder.cpp
    SocialForceModelBuilder.hpp
    SocialForceModelData.hpp
    SocialForceModelUpdate.hpp
)
target_include_directories(simulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})