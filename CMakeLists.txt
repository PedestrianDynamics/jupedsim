cmake_minimum_required(VERSION 2.8)

project(JPSreport)

set(LIBRARY_OUTPUT_PATH ../lib/${CMAKE_BUILD_TYPE})

set(EXECUTABLE_OUTPUT_PATH "../../")

file(
	GLOB_RECURSE
	source_files
	main.cpp
	Simulation.cpp
	
	general/ArgumentParser.cpp  
	general/xmlParser.cpp  

	geometry/Building.cpp  
	geometry/Line.cpp     
	geometry/Point.cpp  
	geometry/Transition.cpp  
	geometry/Building.h    
	geometry/Hline.cpp   
	geometry/Obstacle.cpp  
	geometry/SubRoom.cpp  
	geometry/Crossing.cpp  
	geometry/NavLine.cpp  
	geometry/Room.cpp   
	geometry/Wall.cpp
	
	IO/IODispatcher.cpp  
	IO/OutputHandler.cpp  
	IO/TraVisToClient.cpp  



	header_files:

	general/ArgumentParser.h  
	general/Macros.h  
	general/xmlParser.h
	geometry/Crossing.h  
	geometry/NavLine.h     
	geometry/Room.h       
	geometry/Wall.h
	geometry/Line.h       
	geometry/Point.h    
	geometry/Transition.h
	geometry/Hline.h     
	geometry/Obstacle.h    
	geometry/SubRoom.h    
	IO/IODispatcher.h  
	IO/OutputHandler.h  
	IO/TraVisToClient.h
		
)


#find the correct OpenMP flag
FIND_PACKAGE(OpenMP)
if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

#find and add the CGAL library
FIND_PACKAGE(CGAL)
if(CGAL_FOUND)
#    set (_CGAL 1)
endif()


add_executable(
	JPSreport.exe
	${source_files}
)
