## Generalized Centrifugal Force Model sources
target_sources(simulator PRIVATE
    GeneralizedCentrifugalForceModel.cpp
    GeneralizedCentrifugalForceModel.hpp
    GeneralizedCentrifugalForceModelState.hpp
)
target_include_directories(simulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})