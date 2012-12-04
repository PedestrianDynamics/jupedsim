MCC = g++
#MCC = mpicxx
CC=g++
VTT= vtcxx -vt:cxx mpicxx
CFLAGS=-c -Wall -ansi
LDFLAGS= -fopenmp 
LDEBUG= -g3
LREALEASE= -O3
VERSION=RELEASE
ENABLE_CGAL=TRUE1

ifeq "$(ENABLE_CGAL)" "TRUE"
LIBS= -lCGAL -lgmp
endif

ifeq "$(VERSION)" "DEBUG"
CFLAGS += $(LDEBUG)
else
ifeq "$(VERSION)" "RELEASE"
CFLAGS += $(LREALEASE)
endif
endif


SOURCES=main.cpp Simulation.cpp general/ArgumentParser.cpp general/xmlParser.cpp\
geometry/SubRoom.cpp geometry/Wall.cpp geometry/Transition.cpp geometry/Line.cpp\
geometry/Point.cpp geometry/Room.cpp geometry/Building.cpp geometry/Crossing.cpp\
geometry/Hline.cpp geometry/Obstacle.cpp IO/IODispatcher.cpp IO/TraVisToClient.cpp\
IO/OutputHandler.cpp geometry/NavLine.cpp\
math/Distribution.cpp math/Mathematics.cpp math/ODESolver.cpp math/ForceModel.cpp\
routing/AccessPoint.cpp routing/GlobalRouter.cpp routing/Routing.cpp routing/DummyRouter.cpp\
routing/DirectionStrategy.cpp pedestrian/PedDistributor.cpp pedestrian/Pedestrian.cpp\
pedestrian/Ellipse.cpp mpi/LCGrid.cpp \
routing/QuickestPathRouter.cpp routing/NavMesh.cpp\
routing/GraphRouter.cpp  routing/graph/RoutingGraph.cpp routing/graph/RoutingGraphStorage.cpp\
routing/graph/NavLineState.cpp \
routing/DTriangulation.cpp \
MCD/AlgorithmBase.cpp  MCD/AlgorithmMCD.cpp  MCD/AlgorithmMWT.cpp  MCD/AlgorithmVP.cpp \
MCD/GeomHomog.cpp  MCD/GeomPairDeque.cpp  MCD/GeomPoly.cpp  MCD/GeomVector.cpp \
poly2tri/common/shapes.cpp poly2tri/sweep/sweep_context.cpp \
poly2tri/sweep/advancing_front.cpp  poly2tri/sweep/cdt.cpp  poly2tri/sweep/sweep.cpp  \

OBJECTS=$(SOURCES:.cpp=.o)
DEP=$(SOURCES:.cpp=.d)
EXECUTABLE=rebuild.exe

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(MCC) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

# pull in dependency info for *existing* .o files
-include $(OBJECTS:.o=.d)

.cpp.o:
	$(MCC) $(CFLAGS) $(LDFLAGS) $< -o $@
	$(MCC) -MM $(CFLAGS) $*.cpp > $*.d

release:
	$(MCC) $(LDFLAGS) $(LREALEASE) -o $(EXECUTABLE) $(SOURCES)

vampire:
	$(VTT) $(LDFLAGS) $(LDEBUG) -DVTRACE -o $(EXECUTABLE) $(SOURCES)
	
debug:
	$(MCC) $(LDFLAGS) $(LDEBUG) -o $(EXECUTABLE) $(SOURCES)
	
clean :
	rm -f $(EXECUTABLE) 
	rm -f $(OBJECTS)
	rm -f $(DEP)
	
purge :
	rm -f $(EXECUTABLE) 
	rm -f $(OBJECTS)
	rm -f $(DEP)
	rm trace_*
