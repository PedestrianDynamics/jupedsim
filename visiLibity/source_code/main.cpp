/*
=========A VisiLibity Example Program=========
This program provides a text interface which will

(1) read an environment and guard locations from files given as
command line arguments,

(2) check the geometric validity of these inputs (edges of the
environment do not cross each other, guards are contained in the
environment, etc.),

(3) display environment and guards' data and statistics, and

(4) compute and display the visibility polygon of a guard chosen by
    the user.

The environment representation and the guard locations can be read
from any files in the (human-readable) format demonstrated in
example.environment and example.guards.

Instructions: use the accompanying makefile to compile, then from
command line run
./main [environment file] [guards file]
,e.g., using the example files included with your VisiLibity download,
./main example.environment example.guards
*/

#include "visilibity.hpp"  //VisiLibity header file
#include <cmath>         //Puts math functions in std namespace
#include <cstdlib>       //Gives rand, srand, exit
#include <ctime>         //Gives Unix time
#include <fstream>       //File I/O
#include <iostream>      //std I/O
#include <cstring>       //Gives C-string manipulation
#include <string>        //Gives string class
#include <sstream>       //Gives string streams
#include <vector>        //std vectors
//#define NDEBUG           //Turns off assert.
#include <cassert>


//ASCII escape sequences for colored terminal text.
std::string alert("\a");       //Beep
std::string normal("\x1b[0m"); //Designated fg color default bg color
std::string red("\x1b[31m");         
std::string red_blink("\x1b[5;31m");
std::string black("\E[30;47m");
std::string green("\E[32m");
std::string yellow("\E[33;40m");
std::string blue("\E[34;47m");
std::string magenta("\x1b[35m");
std::string cyan("\E[36m");
std::string white_bold("\E[1;37;40m");
std::string clear_display("\E[2J");
  

//=========================Main=========================//
int main(int argc, char *argv[])
{

  //Check input validity
  if(argc > 3){
    std::cerr << "Error: too many input arguments" << std::endl;
    exit(1);
  }


  //Set iostream floating-point display format
  const int IOS_PRECISION = 10;
  std::cout.setf(std::ios::fixed);
  std::cout.setf(std::ios::showpoint);
  std::cout.precision(IOS_PRECISION);


  //Seed the rand() fnc w/Unix time
  //(only necessary once at the beginning of the program)
  std::srand( std::time( NULL ) ); rand();


  //Set geometric robustness constant
  //:WARNING: 
  //may need to modify epsilon for Environments with greatly varying
  //scale of features
  double epsilon = 0.000000001;
  std::cout << green << "The robustness constant epsilon is set to "
	    << epsilon << normal << std::endl;


  /*----------Load Geometry from Files----------*/


  //Load geometric environment model from file
  std::cout << "Loading environment file ";
  std::string environment_file(argv[1]);
  //Print environment filename to screen
  std::cout << environment_file << " . . . ";
  //Construct Environment object from file
  VisiLibity::Environment my_environment(environment_file);
  std::cout << "OK" << std::endl;


  //Load guard positions from file
  std::cout << "Loading guards file ";
  std::string guards_file(argv[2]);
  //Print guards filename to screen
  std::cout << guards_file << " . . . ";
  //Construct Guards object from file
  VisiLibity::Guards my_guards(guards_file);
  std::cout << "OK" << std::endl;


  /*----------Check Validity of Geometry----------*/


  //Check Environment is epsilon-valid
  std::cout << "Validating environment model . . . ";
  if(  my_environment.is_valid( epsilon )  )
    std::cout << "OK" << std::endl;
  else{
    std::cout << std::endl << red << "Warning:  Environment model "
	      << "is invalid." << std::endl
	      << "A valid environment model must have" << std::endl
	      << "   1) outer boundary and holes pairwise "
	      << "epsilon -disjoint simple polygons" << std::endl
	      << "   (no two features should come "
	      << "within epsilon of each other)," << std::endl 
	      << "   2) outer boundary is oriented ccw, and" 
	      << std::endl
	      << "   3) holes are oriented cw."
	      << std::endl
	      << normal; 
    exit(1);
  }


  //Check Guards are all in the Environment
  std::cout << "Checking all guards are "
	    << "in the environment and noncolocated . . . ";
  my_guards.snap_to_boundary_of(my_environment, epsilon);
  my_guards.snap_to_vertices_of(my_environment, epsilon);
  for(unsigned i=0; i<my_guards.N(); i++){
    if( !my_guards[i].in(my_environment, epsilon) ){
      std::cout << std::endl << red 
		<< "Warning:  guard " << i 
		<< " not in the environment."
		<< normal << std::endl;
      exit(1);
    }
  }
  if( !my_guards.noncolocated(epsilon) ){
    std::cout << std::endl << red 
	      << "Warning:  Some guards are colocated." 
	      <<  normal << std::endl;
    exit(1);
  }
  else
    std::cout << "OK" << std::endl;



  /*----------Print Data and Statistics to Screen----------*/


  //Environment data
  std::cout << "The environment model is:" << std::endl;
  std::cout << magenta << my_environment << normal;


  //Environment stats
  std::cout << "This environment has " << cyan 
	    << my_environment.n() << " vertices, " 
	    << my_environment.r() << " reflex vertices, " 
	    << my_environment.h() << " holes, "
	    << "area " << my_environment.area() << ", "
	    << "boundary length " 
	    << my_environment.boundary_length() << ", "
	    << "diameter " 
	    << my_environment.diameter() << "."
	    << normal << std::endl;


  //Guards data
  std::cout << "The guards' positions are:" << std::endl;
  std::cout << magenta << my_guards << normal;


  //Guards stats
  std::cout << "There are " << cyan << my_guards.N() 
	    << " guards." << normal << std::endl;


  /*----------Compute the Visibility Polygon 
                   of a Guard Chosen by User----------*/
 
 
  //Prompt user
  int guard_choice(0);
  std::cout << "Which guard would you like "
	    <<"to compute the visibility polygon of "
	    << "(0, 1, 2, ...)? " << std::endl;
  std::cin >> guard_choice; std::cout << normal;

	 
  //Compute and display visibility polygon
  VisiLibity::Visibility_Polygon
    my_visibility_polygon(my_guards[guard_choice], my_environment, epsilon);
  std::cout << "The visibility polygon is" << std::endl
	    << magenta << my_visibility_polygon << normal
	    << std::endl;

  /*
  //To save the visibility polygon in an Environment file
  VisiLibity::Environment(my_visibility_polygon)
    .write_to_file("./example_visibility_polygon.cin", IOS_PRECISION);
  */

  return 0;
}
