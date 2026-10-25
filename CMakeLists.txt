cmake_minimum_required(VERSION 3.16)
project(Hyperball LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

add_executable(hyperball main.cpp hyperball.cpp)
target_link_libraries(hyperball PRIVATE sfml-graphics sfml-window sfml-system)
