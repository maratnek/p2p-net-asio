add_library(net STATIC
       server.cpp
       session.cpp
)

target_link_libraries(net 
       utils
       CONAN_PKG::boost 
       CONAN_PKG::logger 
)

# Install the library
install(TARGETS net DESTINATION lib)

# Include directories for the library
target_include_directories(net PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
