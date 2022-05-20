cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(JuPedSimLibraryTest VERSION 1.0.0 LANGUAGES C)

find_package(jupedsim REQUIRED)

add_executable(JuPedSimLibraryTest
    main.c
)
target_link_libraries(JuPedSimLibraryTest
    jupedsim::jupedsim
)
