cmake_minimum_required(VERSION 3.0.0)
project(node-network VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_REQUIRED ON)

include(CTest)
enable_testing()


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})


# set(CMAKE_CXX_COMPILER /usr/bin/clang++)
# set(CMAKE_GENERATOR Xcode)
# set(CONAN_DISABLE_CHECK_COMPILER ON)


# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
                "${CMAKE_BINARY_DIR}/conan.cmake"
                TLS_VERIFY ON)
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)
include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
# conan_basic_setup(TARGETS)
conan_cmake_run(REQUIRES rapidjson/cci.20220822@ 
                         boost/1.83.0@
                        #  benchmark/1.6.0@ 
                        #  leveldb/1.23@
                        #  protobuf/3.21.12@
                        #  spdlog/1.12.0@
                        # poco/1.9.4@
        # OPTIONS Poco::building=True
        GENERATORS cmake cmake_multi cmake_find_package cmake_find_package_multi cmake_paths 
        # GENERATORS cmake CMakeDeps CMakeToolchain
        # BUILD missing
        # PROFILE opengl 
        # PROFILE clang12release 
        # SETTINGS compiler.version=12.0
        # BUILD_TYPE Debug
        BASIC_SETUP CMAKE_TARGETS
        # REMOTE conancenter
)

find_package(RapidJSON REQUIRED)
# find_package(Boost 1.83 REQUIRED COMPONENTS system)
find_package(Boost 1.83 REQUIRED COMPONENTS system)

# find_package(Protobuf REQUIRED)
# set(PROTO_FILES
#        proto/data.proto
#        )

# include_directories(${CMAKE_CURRENT_BINARY_DIR})
# protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

# add_executable(processing 
#        main.cpp 
#        ${PROTO_SRCS}
# )

# target_link_libraries(processing 
#        CONAN_PKG::leveldb 
#        CONAN_PKG::protobuf
#        CONAN_PKG::spdlog 
# )

add_executable(p2p-test 
        p2p-client-test.cpp
        session.cpp
        server.cpp
)

target_link_libraries(p2p-test 
        CONAN_PKG::rapidjson
        CONAN_PKG::boost
)

add_executable(client-test 
        client-test.cpp
        session.cpp
        server.cpp
)
target_link_libraries(client-test 
        CONAN_PKG::rapidjson
        CONAN_PKG::boost
)
