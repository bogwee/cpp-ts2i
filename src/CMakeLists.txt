cmake_minimum_required(VERSION 3.5)
project(codec_cpp)

find_package(OpenCV REQUIRED)
find_package(Eigen3 REQUIRED)

add_executable(codec main.cpp)
target_include_directories(codec PRIVATE ${EIGEN3_INCLUDE_DIR})
target_link_libraries(codec PRIVATE ${OpenCV_LIBS})
