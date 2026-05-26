## Generalized Centrifugal Force Model sources
list(APPEND SIMULATOR_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/GeneralizedCentrifugalForceModel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GeneralizedCentrifugalForceModel.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GeneralizedCentrifugalForceModelBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GeneralizedCentrifugalForceModelBuilder.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GeneralizedCentrifugalForceModelData.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GeneralizedCentrifugalForceModelUpdate.hpp
)

list(APPEND SIMULATOR_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR})

set(SIMULATOR_SOURCES ${SIMULATOR_SOURCES} PARENT_SCOPE)
set(SIMULATOR_INCLUDE ${SIMULATOR_INCLUDE} PARENT_SCOPE)