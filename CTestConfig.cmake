## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.
##
## # The following are required to submit to the CDash dashboard:
##   ENABLE_TESTING()
##   INCLUDE(CTest)


set(CTEST_PROJECT_NAME "JuPedSim")
#set(CTEST_NIGHTLY_START_TIME "00:00:00 GMT")

set(CTEST_DROP_METHOD "http")
set(CTEST_DROP_SITE "my.cdash.org")
set(CTEST_DROP_LOCATION "/submit.php?project=JuPedSim")
set(CTEST_DROP_SITE_CDASH TRUE)

set(WITH_MEMCHECK TRUE)

find_program(CTEST_GIT_COMMAND NAMES git)
find_program(CTEST_MEMORYCHECK_COMMAND NAMES valgrind)



# set(CTEST_UPDATE_COMMAND "${CTEST_GIT_COMMAND}")




# ctest using git
# set(CTEST_SOURCE_DIRECTORY "$ENV{HOME}/workspace/tmp/dashboards/libssh/source")
# set(CTEST_BINARY_DIRECTORY "$ENV{HOME}/workspace/tmp/dashboards/libssh/build")
# set(CTEST_BUILD_NAME "linux-gcc-default")

# set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
# set(CTEST_BUILD_CONFIGURATION "Profiling")
# set(CTEST_BUILD_OPTIONS "-DWITH_SSH1=ON -WITH_SFTP=ON -DWITH_SERVER=ON -DWITH_ZLIB=ON -DWITH_PCAP=ON -DWITH_GCRYPT=OFF")



# #######################################################################

# ctest_empty_binary_directory(${CTEST_BINARY_DIRECTORY})


# find_program(CTEST_COVERAGE_COMMAND NAMES gcov)

#if (WITH_MEMCHECK)

#set(CTEST_NIGHTLY_START_TIME "00:00:00 GMT")
  
# set(CTEST_MEMORYCHECK_SUPPRESSIONS_FILE ${CTEST_SOURCE_DIRECTORY}/tests/valgrind.supp)

# if(NOT EXISTS "${CTEST_SOURCE_DIRECTORY}")
#   set(CTEST_CHECKOUT_COMMAND "${CTEST_GIT_COMMAND} clone git://git.libssh.org/projects/libssh/libssh.git ${CTEST_SOURCE_DIRECTORY}")
# endif()

# set(CTEST_UPDATE_COMMAND "${CTEST_GIT_COMMAND}")

# set(CTEST_CONFIGURE_COMMAND "${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE:STRING=${CTEST_BUILD_CONFIGURATION}")
# set(CTEST_CONFIGURE_COMMAND "${CTEST_CONFIGURE_COMMAND} -DWITH_TESTING:BOOL=ON ${CTEST_BUILD_OPTIONS}")
# set(CTEST_CONFIGURE_COMMAND "${CTEST_CONFIGURE_COMMAND} \"-G${CTEST_CMAKE_GENERATOR}\"")
# set(CTEST_CONFIGURE_COMMAND "${CTEST_CONFIGURE_COMMAND} \"${CTEST_SOURCE_DIRECTORY}\"")

#ctest_start("Nightly")
# ctest_update()
# ctest_configure()
#ctest_build()
# ctest_test()
# if (WITH_COVERAGE AND CTEST_COVERAGE_COMMAND)
#   ctest_coverage()
# endif (WITH_COVERAGE AND CTEST_COVERAGE_COMMAND)
# if (WITH_MEMCHECK AND CTEST_MEMORYCHECK_COMMAND)
#   ctest_memcheck( res )
# endif (WITH_MEMCHECK AND CTEST_MEMORYCHECK_COMMAND)

# ctest_submit()