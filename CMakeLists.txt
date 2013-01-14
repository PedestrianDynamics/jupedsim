cmake_minimum_required(VERSION 2.8)

project(JPSgcfm)

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

	math/Distribution.cpp  
	math/ForceModel.cpp  
	math/Mathematics.cpp  
	math/ODESolver.cpp  

	mpi/LCGrid.cpp  

	pedestrian/Ellipse.cpp  
	pedestrian/PedDistributor.cpp  
	pedestrian/Pedestrian.cpp  

	routing/AccessPoint.cpp  
	routing/DirectionStrategy.cpp  
	routing/DummyRouter.cpp  
	routing/GlobalRouter.cpp  
	routing/GraphRouter.cpp  
	routing/QuickestPathRouter.cpp  
	routing/Router.cpp
	routing/RoutingEngine.cpp
	routing/NavMesh.cpp
	routing/DTriangulation.cpp    
	
	routing/graph/NavLineState.cpp  
	routing/graph/NavLineState.h  
	routing/graph/RoutingGraph.cpp  
	routing/graph/RoutingGraph.h  
	routing/graph/RoutingGraphStorage.cpp  
	routing/graph/RoutingGraphStorage.h

	poly2tri/common/shapes.cpp          
	poly2tri/sweep/sweep_context.cpp
	poly2tri/sweep/advancing_front.cpp  
	poly2tri/sweep/sweep.cpp
	poly2tri/sweep/cdt.cpp

	header_files:
	routing/NavMesh.ch
	routing/DirectionStrategy.h    
	routing/DummyRouter.h    
	routing/GlobalRouter.h    
	routing/GraphRouter.h    
	routing/QuickestPathRouter.h    
	routing/Router.h
	routing/RoutingEngine.h
	routing/AccessPoint.h    
	routing/DTriangulation.h    
	pedestrian/Pedestrian.h
	pedestrian/PedDistributor.h  
	pedestrian/Ellipse.h  
	mpi/LCGrid.h
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
	math/ForceModel.h  
	math/Distribution.h  
	math/Mathematics.h  
	math/ODESolver.h
	poly2tri/poly2tri.h
	poly2tri/common/shapes.h          
	poly2tri/sweep/cdt.h
	poly2tri/common/utils.h           
	poly2tri/sweep/sweep_context.h
	poly2tri/sweep/advancing_front.h  
	poly2tri/sweep/sweep.h
	
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
	rebuild.exe
	${source_files}
)
