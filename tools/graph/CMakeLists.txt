add_executable(graph
    main.cpp
)
target_link_libraries(graph PRIVATE
    Boost::boost
)
