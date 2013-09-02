cmake_minimum_required(VERSION 2.8)

project(JPSreport)

if(WIN32)
#Set the default compiler to MinGW instead of NMake
#set(-G "MinGW Makefiles")
endif()

set(LIBRARY_OUTPUT_PATH ../lib/${CMAKE_BUILD_TYPE})

#set(EXECUTABLE_OUTPUT_PATH "../")
INCLUDE_DIRECTORIES("./")

file(
	GLOB_RECURSE
	source_files
	Analysis.cpp
	main.cpp
	VoronoiDiagram.cpp
	VoronoiDiagramGenerator.cpp
	VoronoiPolygons.cpp
	IO/IODispatcher.cpp
	IO/OutputHandler.cpp
	general/ArgumentParser.cpp
	tinyxml/tinystr.cpp
	tinyxml/tinyxml.cpp
	tinyxml/tinyxmlerror.cpp
	tinyxml/tinyxmlparser.cpp
	geometry/Building.cpp  
	geometry/Line.cpp      
	geometry/Point.cpp    
	geometry/Transition.cpp
	geometry/Crossing.cpp  
	geometry/NavLine.cpp   
	geometry/Room.cpp     
	geometry/Wall.cpp
	geometry/Hline.cpp     
	geometry/Obstacle.cpp  
	geometry/SubRoom.cpp

	header_files:
	Analysis.h
	MeasurementArea.h  
	VoronoiDiagramGenerator.h  
	VoronoiDiagram.h  
	VoronoiPolygons.h
	IO/IODispatcher.h
	IO/OutputHandler.h
	general/ArgumentParser.h
	tinyxml/tinyxml.h
	tinyxml/tinystr.h
	general/Macros.h
	geometry/Building.h  
	geometry/Line.h      
	geometry/Point.h    
	geometry/Transition.h
	geometry/Crossing.h  
	geometry/NavLine.h   
	geometry/Room.h     
	geometry/Wall.h
	geometry/Hline.h     
	geometry/Obstacle.h  
	geometry/SubRoom.h

)


#find the correct OpenMP flag
#FIND_PACKAGE(OpenMP)
#if(OPENMP_FOUND)
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
#endif()


#message(source_files)

add_executable(
	report.exe
	${source_files}
)

if(WIN32)
target_link_libraries (report.exe wsock32)
endif()

#ADD_CUSTOM_COMMAND(TARGET rebuild
#          POST_BUILD
#          COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}
#          COMMAND ${CMAKE_COMMAND} -E copy ${EXAMPLE_BIN_NAME} ${PROJECT_BINARY_DIR}/.
#)
