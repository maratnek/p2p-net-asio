add_executable(p2p-test p2p-test.cpp)

target_link_libraries(p2p-test 
       PRIVATE net
)
