cmake_minimum_required(VERSION 3.10)

project(Happy_Mapper VERSION 0.0.1)



add_executable(Happy_Mapper mapper.cpp)

find_package(X11)
target_link_libraries(Happy_Mapper X11)

