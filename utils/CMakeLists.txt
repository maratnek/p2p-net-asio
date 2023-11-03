cmake_minimum_required(VERSION 3.0.0)
project(net VERSION 0.1.0 LANGUAGES C CXX)

add_library(net STATIC
       server.cpp
       session.cpp
)

# Include directories for the library
target_include_directories(net PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


target_link_libraries(net 
       CONAN_PKG::boost 
       CONAN_PKG::logger 
       CONAN_PKG::rapidjson
       CONAN_PKG::protobuf
)
