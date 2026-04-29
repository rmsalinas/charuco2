cmake_minimum_required(VERSION 3.10)
project(testcharuco)

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Include OpenCV headers
include_directories(${OpenCV_INCLUDE_DIRS})

# Add the executable
add_executable(testcharuco testcharuco.cpp charuco2.cpp charuco2.h)

# Link OpenCV libraries
target_link_libraries(testcharuco ${OpenCV_LIBS})
