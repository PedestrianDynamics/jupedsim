cmake_minimum_required (VERSION 2.8  FATAL_ERROR)

set (BDIR "/usr/bin")

set (CTEST_PROJECT_NAME "JuPedSim")
set (CTEST_BUILD_NAME "linux-gcc-zam")
set (CTEST_TIMEOUT "1500") # max run time for tests 1500 s 
set (CTEST_BACKUP_AND_RESTORE TRUE)
set (CTEST_SOURCE_DIRECTORY "$ENV{HOME}/workspace/peddynamics/JuPedSim/JPScore/trunk")
set (CTEST_BINARY_DIRECTORY "$ENV{HOME}/workspace/peddynamics/JuPedSim/JPScore/trunk/build")

#set (CTEST_NIGHTLY_START_TIME "00:00:00 CET")
#find_program (HOSTNAME_CMD NAMES hostname)
#exec_program (${HOSTNAME_CMD} ARGS OUTPUT_VARIABLE HOSTNAME)
set (CTEST_SITE  "Workspace")
set (CTEST_DROP_METHOD "http")
set (CTEST_DROP_SITE "my.cdash.org")
set (CTEST_DROP_LOCATION "/submit.php?project=JuPedSim")
set (CTEST_DROP_SITE_CDASH TRUE)
set (CTEST_CMAKE_GENERATOR "Unix Makefiles")
set(CTEST_CUSTOM_MAXIMUM_NUMBER_OF_WARNINGS 1000)
#set (CTEST_BUILD_CONFIGURATION "Debug")
set (CTEST_CONFIGURATION_TYPE "Debug")

#----------------------------------------------------------------------
if(NOT DEFINED PROCESSOR_COUNT)
  # Unknown:
  set(PROCESSOR_COUNT 0)
  # Linux:
  set(cpuinfo_file "/proc/cpuinfo")
  if(EXISTS "${cpuinfo_file}")
    file(STRINGS "${cpuinfo_file}" procs REGEX "^processor.: [0-9]+$")
    list(LENGTH procs PROCESSOR_COUNT)
  endif(EXISTS "${cpuinfo_file}")
endif(NOT DEFINED PROCESSOR_COUNT)

if(PROCESSOR_COUNT)
  # add 1 should be magic! http://www.kitware.com/blog/home/post/63
  ##math(EXPR PROCESSOR_COUNT "${PROCESSOR_COUNT} + 1") 
  message("PROCESSOR_COUNT " ${PROCESSOR_COUNT})
  set(CTEST_BUILD_FLAGS "-j${PROCESSOR_COUNT}")
endif(PROCESSOR_COUNT)
#----------------------------------------------------------------------

# Errors that will be ignored
set(CTEST_CUSTOM_ERROR_EXCEPTION
  ${CTEST_CUSTOM_ERROR_EXCEPTION}
  "ICECC"
  "Segmentation fault"
  "GConf Error"
  "Client failed to connect to the D-BUS daemon"
  "Failed to connect to socket"
  )

# No coverage for these files (auto-generated, unit tests, etc)
set(CTEST_CUSTOM_COVERAGE_EXCLUDE ".moc$" "moc_" "ui_" "${CTEST_SOURCE_DIRECTORY}/Utest"  "qrc_")

#set (WITH_MEMCHECK TRUE)
set (WITH_MEMCHECK FALSE)

#if(CMAKE_COMPILER_IS_GNUCXX)
set(WITH_COVERAGE TRUE)
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -W -Wshadow -Wunused-variable -Wunused-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecated -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage")
#set(CMAKE_CXX_LDFLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
#message("Debug flags for coverage: " ${CMAKE_CXX_FLAGS_DEBUG} )
##endif(CMAKE_COMPILER_IS_GNUCXX)


#set (CTEST_CMAKE_COMMAND "${BDIR}/cmake")
find_program (CTEST_CMAKE_COMMAND NAMES cmake)
#find_program(CTEST_GIT_COMMAND NAMES git) # later for GitHub
find_program (CTEST_SVN_COMMAND NAMES svn)
find_program (CTEST_MEMORYCHECK_COMMAND NAMES valgrind)
find_program (CTEST_COVERAGE_COMMAND NAMES gcov)
#set (CTEST_COVERAGE_COMMAND "${BDIR}/gcov")
#set(CTEST_UPDATE_COMMAND "${BDIR}/svn")
set (CTEST_UPDATE_COMMAND "${CTEST_SVN_COMMAND}") # later CTEST_GIT_COMMAND
set (CTEST_BUILD_COMMAND "${BDIR}/make -j${PROCESSOR_COUNT}")

# not necessary since we are using cmake
#set (CTEST_CONFIGURE_COMMAND "${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE:STRING=${CTEST_BUILD_CONFIGURATION}")
#set (CTEST_CONFIGURE_COMMAND "${CTEST_CONFIGURE_COMMAND} -DWITH_TESTING:BOOL=ON ${CTEST_BUILD_OPTIONS}")
#set (CTEST_CONFIGURE_COMMAND "${CTEST_CONFIGURE_COMMAND} \"-G${CTEST_CMAKE_GENERATOR}\"")
#set (CTEST_CONFIGURE_COMMAND "${CTEST_CONFIGURE_COMMAND} \"${CTEST_SOURCE_DIRECTORY}\"")

if (NOT EXISTS "${CTEST_SOURCE_DIRECTORY}")
  #set(CTEST_CHECKOUT_COMMAND "${CTEST_GIT_COMMAND} clone git://git.libssh.org/projects/libssh/libssh.git ${CTEST_SOURCE_DIRECTORY}")
  set (CTEST_CHECKOUT_COMMAND "${CTEST_SVN_COMMAND} co https://svn.version.fz-juelich.de/peddynamics/JuPedSim/JPScore/trunk ${CTEST_SOURCE_DIRECTORY}")
endif ()
#setCTEST_BUILD_OPTIONS "-DWITH_SSH1=ON -WITH_SFTP=ON -DWITH_SERVER=ON -DWITH_ZLIB=ON -DWITH_PCAP=ON -DWITH_GCRYPT=OFF")
set ( $ENV{LC_MESSAGES}    "en_EN" )
# should ctest wipe the binary tree before running
set (CTEST_START_WITH_EMPTY_BINARY_DIRECTORY TRUE)

#######################################################################
ctest_empty_binary_directory(${CTEST_BINARY_DIRECTORY})
# get the party started
ctest_start ("Experimental")
# specify how long to run the continuous in minutes
SET (CTEST_CONTINUOUS_DURATION 650)
SET (CTEST_CONTINUOUS_MINIMUM_INTERVAL 15)

ctest_update (SOURCE "${CTEST_SOURCE_DIRECTORY}" RETURN_VALUE res)

ctest_configure (BUILD "${CTEST_BINARY_DIRECTORY}" 
  SOURCE   ${CTEST_SOURCE_DIRECTORY}
  OPTIONS   "-DCMAKE_BUILD_TYPE:STRING=Debug"
  RETURN_VALUE res)

ctest_build (BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res)

ctest_test (BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res)

if (WITH_MEMCHECK AND CTEST_MEMORYCHECK_COMMAND)
  ctest_memcheck(RETURN_VALUE res)
endif (WITH_MEMCHECK AND CTEST_MEMORYCHECK_COMMAND)

if (WITH_COVERAGE AND CTEST_COVERAGE_COMMAND)
  #ctest_coverage(BUILD  ${CTEST_SOURCE_DIRECTORY}/Utest RETURN_VALUE res)
  ctest_coverage() #BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res)
  message ("ctest_coverage() return: " ${res} )
endif (WITH_COVERAGE AND CTEST_COVERAGE_COMMAND)


ctest_submit(RETURN_VALUE res)

#ctest_empty_binary_directory (${CTEST_BINARY_DIRECTORY})

#######################################################################
message ("DONE!")
