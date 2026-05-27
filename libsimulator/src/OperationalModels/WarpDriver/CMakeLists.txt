## Generalized Centrifugal Force Model sources
target_sources(simulator PRIVATE
    WarpDriverModel.cpp
    WarpDriverModel.hpp
    WarpDriverModelBuilder.cpp
    WarpDriverModelBuilder.hpp
    WarpDriverModelData.hpp
    WarpDriverModelUpdate.hpp
)
target_include_directories(simulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})