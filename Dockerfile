# Use a base image with your desired Linux distribution and C++ environment
# FROM conanio/gcc9:latest
FROM ubuntu:latest

# Copy your source code and Conan recipe into the container
# COPY conanfile.txt /app/conanfile.txt
# COPY node.cpp /app/node.cpp
COPY ./build/bin/node-network /app/node-network
COPY ./config.json /app/config.json

# Set the working directory
WORKDIR /app

# Install Conan packages
# RUN conan install . -s compiler.libcxx=libstdc++11

# Compile the code
# RUN g++ -o node node.cpp $(conan_cmake)
