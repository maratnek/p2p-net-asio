add_executable(client-test client-test.cpp)
target_link_libraries(client-test 
       PRIVATE net
)

# Include directories for the library
target_include_directories(net PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
