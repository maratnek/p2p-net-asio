add_library(utils STATIC config.cpp) 

# Include directories for the library
target_include_directories(utils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(utils 
       CONAN_PKG::rapidjson
       CONAN_PKG::logger 
)

# Install the headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION include)