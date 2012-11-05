cmake_minimum_required(VERSION 2.8)

project(JPSgcfm)

set(LIBRARY_OUTPUT_PATH ../lib/${CMAKE_BUILD_TYPE})

set(EXECUTABLE_OUTPUT_PATH "./")

file(
	GLOB_RECURSE
	source_files
	main.cpp
	Simulation.cpp
	*.h
	
	general/ArgumentParser.cpp  
	general/ArgumentParser.h  
	general/Macros.h  
	general/xmlParser.cpp  
	general/xmlParser.h
	
	geometry/Building.cpp  
	geometry/Crossing.h  
	geometry/Line.cpp     
	geometry/NavLine.h     
	geometry/Point.cpp  
	geometry/Room.h       
	geometry/Transition.cpp  
	geometry/Wall.h
	geometry/Building.h    
	geometry/Hline.cpp   
	geometry/Line.h       
	geometry/Obstacle.cpp  
	geometry/Point.h    
	geometry/SubRoom.cpp  
	geometry/Transition.h
	geometry/Crossing.cpp  
	geometry/Hline.h     
	geometry/NavLine.cpp  
	geometry/Obstacle.h    
	geometry/Room.cpp   
	geometry/SubRoom.h    
	geometry/Wall.cpp
	
	IO/IODispatcher.cpp  
	IO/IODispatcher.h  
	IO/OutputHandler.cpp  
	IO/OutputHandler.h  
	IO/TraVisToClient.cpp  
	IO/TraVisToClient.h

	math/Distribution.cpp  
	math/Distribution.h  
	math/ForceModel.cpp  
	math/ForceModel.h  
	math/Mathematics.cpp  
	math/Mathematics.h  
	math/ODESolver.cpp  
	math/ODESolver.h

	mpi/LCGrid.cpp  
	mpi/LCGrid.h

	pedestrian/Ellipse.cpp  
	pedestrian/Ellipse.h  
	pedestrian/PedDistributor.cpp  
	pedestrian/PedDistributor.h  
	pedestrian/Pedestrian.cpp  
	pedestrian/Pedestrian.h

	routing/AccessPoint.cpp  
	routing/DirectionStrategy.cpp  
	routing/DummyRouter.cpp  
	routing/GlobalRouter.cpp  
	routing/GraphRouter.cpp  
	routing/QuickestPathRouter.cpp  
	routing/Routing.cpp
	routing/AccessPoint.h    
	routing/DirectionStrategy.h    
	routing/DummyRouter.h    
	routing/GlobalRouter.h    
	routing/GraphRouter.h    
	routing/QuickestPathRouter.h    
	routing/Routing.h
	
	routing/graph/NavLineState.cpp  
	routing/graph/NavLineState.h  
	routing/graph/RoutingGraph.cpp  
	routing/graph/RoutingGraph.h  
	routing/graph/RoutingGraphStorage.cpp  
	routing/graph/RoutingGraphStorage.h

    *.h
    */*.h
)


#find the correct OpenMP flag
FIND_PACKAGE(OpenMP)
if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

add_executable(
	rebuild.exe
	${source_files}
)


