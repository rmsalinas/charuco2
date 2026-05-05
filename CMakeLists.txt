cmake_minimum_required(VERSION 3.10)
project(charuco2)

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Include OpenCV headers
include_directories(${OpenCV_INCLUDE_DIRS})

#treat warnings as errors
add_compile_options(-Wall -Wextra -Werror)

# Add the executable
add_executable(detectboard detectboard.cpp charuco2.cpp charuco2.h)
target_link_libraries(detectboard   ${OpenCV_LIBS})
add_executable(detectdiamonds detectdiamonds.cpp charuco2.cpp charuco2.h)
target_link_libraries(detectdiamonds   ${OpenCV_LIBS})

