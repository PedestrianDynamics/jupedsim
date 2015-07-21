/*
=========A VisiLibity Example Program=========

This MEX-file source code is for computing a shortest path (of a
point) through a polygonal environment with holes in Matlab using the
Environment::shortest_path() method of VisiLibity.

To compile, first make visilibity.o if it has not already been made.
Then set up your MEX compiler at the Matlab prompt using the command
>>mex -setup
, and finally to generate the MEX file at the Matlab prompt run
>> mex -v shortest_path.cpp visilibity.o
.  To call the MEX-file from the Matlab prompt, use
>>shortest_path(start, finish, environment, epsilon, snap_distance)
, where
start is a 1x2 array (x and y coordinates),
finish is a 1x2 array (x and y coordinates),
environment is a cell array of the environment vertices
(x and y coordinates),
epsilon is a double (the robustness constant), and
snap_distance is a double representing the largest distance at which
the start and finish Poinst will still be snapped to the vertices and 
boundary of the environment.
*/


//Gives Matlab interfacing.  Linkage block specifies linkage
//convention used to link header file; makes the C header suitable for
//C++ use.  "mex.h" also includes "matrix.h" to provide MX functions
//to support Matlab data types, along with the standard header files
//<stdio.h>, <stdlib.h>, and <stddef.h>.
extern "C" {
#include <math.h>
#include "mex.h"
}


#include "visilibity.hpp"  //VisiLibity header file
//#include <cmath>         //Puts math functions in std namespace
#include <cstdlib>       //rand, srand, exit
#include <ctime>         //Unix time
#include <fstream>       //File I/O
#include <iostream>      //std I/O
#include <cstring>       //C-string manipulation
#include <string>        //string class
#include <sstream>       //string streams
#include <vector>        //std vectors
//#define NDEBUG           //Turns off assert.
#include <cassert>


//=========================Main=========================//
//nlhs contains the number of LHS (output) arguments and corresponds
//to the number returned from the Matlab nargout function.  plhs[] is
//an array of pointers to the output arguments (mxArrays).  nrhs
//contains the number or RHS (input arguments).  prhs[] is an array of
//pointers to the input arguments (mxArrays), i.e. prhs[0] points to
//first input argument, prhs[nrhs-1] to last input argument, etc..
//When the function is called, prhs contains pointers to the input
//arguments, while plhs contains null pointers.  Must create output
//arrays and assign pointers to the plhs pointer array.
void mexFunction( int nlhs, mxArray *plhs[],
		  int nrhs, const mxArray *prhs[] )
{

  //Construct the start Point
  double *in0 = mxGetPr( prhs[0] );
  double start_x = in0[0];
  double start_y = in0[1];
  VisiLibity::Point start(start_x, start_y);


  //Construct the finish Point
  double *in1 = mxGetPr( prhs[1] );
  double finish_x = in1[0];
  double finish_y = in1[1];
  VisiLibity::Point finish(finish_x, finish_y);


  //Construct environment
  //Passing structures and cell arrays into MEX-files is just like
  //passing any other data types, except the data itself is of type
  //mxArray. In practice, this means that mxGetField (for structures)
  //and mxGetCell (for cell arrays) return pointers of type
  //mxArray. You can then treat the pointers like any other pointers
  //of type mxArray, but if you want to pass the data contained in the
  //mxArray to a C routine, you must use an API function such as
  //mxGetData to access it.
  //
  //Find the dimensions of input
  //int m = mxGetM(prhs[1]);
  //int n = mxGetN(prhs[1]);


  //Auxiliary vars
  //Recall const My_type *my_ptr => my_ptr points to a constant of
  //type My_type, i.e., my_ptr is not a constant pointer to a variable
  //of type My_type.
  const mxArray *polygon_ptr;
  int num_of_coords;
  //coord_ptr is indexable, e.g., my_coord[0] returns the x
  //coordinate of the first vertex.
  const double *coord_ptr;
  std::vector<VisiLibity::Point> vertices;
  VisiLibity::Polygon polygon_temp;


  //Outer Boundary
  polygon_ptr = mxGetCell(prhs[2], 0);
  num_of_coords = mxGetM(polygon_ptr);
  //mexPrintf("The outer boundary has %i vertices.\n", num_of_coords); 
  coord_ptr = mxGetPr( polygon_ptr );
  for(int j=0; j<num_of_coords; j++){
    vertices.push_back(  VisiLibity::Point( coord_ptr[j],
					    coord_ptr[num_of_coords + j] )  );
  }
  //VisiLibity::Environment my_environment( VisiLibity::Polygon(vertices) );
  polygon_temp.set_vertices(vertices);
  VisiLibity::Environment *my_environment;
  my_environment = new VisiLibity::Environment(polygon_temp);
  vertices.clear();


  //Holes  
  //The number of polygons is the number of cells (one polygon
  //per cell).
  int num_of_polygons = mxGetNumberOfElements( prhs[2] );
  for(int i=1; i<num_of_polygons; i++){

    polygon_ptr = mxGetCell(prhs[2], i);
    num_of_coords = mxGetM(polygon_ptr);
    //mexPrintf("The hole #%i has %i vertices.\n", i, num_of_coords); 
    coord_ptr = mxGetPr( polygon_ptr );
    for(int j=0; j<num_of_coords; j++){
      vertices.push_back(  VisiLibity::Point( coord_ptr[j],
				  coord_ptr[num_of_coords + j] )  );
    } 
    
    polygon_temp.set_vertices( vertices );
    my_environment->add_hole( polygon_temp );
    //:COMPILER:
    //Why doesn't this work if my_environment were not a pointer?
    //my_environment.add_hole( polygon_temp );
    vertices.clear();
  }


  //Get robustness constant
  double epsilon = mxGetScalar(prhs[3]);
 
  
  //Adjust start and finish location according to snap distance
  double snap_distance = mxGetScalar(prhs[4]);
  if(  !start.in( *my_environment , epsilon )  )
    start = start.projection_onto_boundary_of( *my_environment );
  start.snap_to_boundary_of( (*my_environment) , snap_distance);
  start.snap_to_vertices_of( (*my_environment) , snap_distance);
  if(  !finish.in( *my_environment , epsilon )  )
    finish = finish.projection_onto_boundary_of( *my_environment );
  finish.snap_to_boundary_of( (*my_environment) , snap_distance);
  finish.snap_to_vertices_of( (*my_environment) , snap_distance);


  //Compute shortest path through environment from start to finish
  VisiLibity::Polyline my_shortest_path;
  my_shortest_path = my_environment->shortest_path(start, finish, epsilon);

  
  //Create an mxArray for the output
  plhs[0] = mxCreateDoubleMatrix( my_shortest_path.size(), 2, mxREAL );
  //Create a pointer to output
  double *out = mxGetPr(plhs[0]);
  //Populate the output
  for (int i=0; i<my_shortest_path.size(); i++){
    out[i]                           = my_shortest_path[i].x();
    out[my_shortest_path.size() + i] = my_shortest_path[i].y();
  }
 

  delete my_environment;

  return;
}
