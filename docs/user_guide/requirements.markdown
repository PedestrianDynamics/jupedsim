
# Requirements

1. [CMake](http://www.cmake.org/)
2. A compiler with support for C++11 e.g. [g++](https://gcc.gnu.org/projects/cxx0x.html) or [clang](http://clang.llvm.org/) or [Visual Studio 2013](http://msdn.microsoft.com/de-de/library/dd831853.aspx)
3. for JPSvis and JPSeditor additionally [Qt](http://qt-project.org/doc/qt-4.8/installation.html) and [VTK](http://www.vtk.org/VTK/resources/software.html) are needed.
4. JPSreport and JPScore need [Boost-libraries](http://www.boost.org/). At the moment boost is hard-linked in JPSreport, but this should be fixed in the future.

JuPedSim offers a Vagrant image that comes with all the necessary
packages and libraries. See for more information [here](Vagrant).
