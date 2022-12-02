cmake_minimum_required(VERSION 3.23)
project(lines)

set(CMAKE_CXX_STANDARD 17)

add_executable(lines main.cpp Sidebar.h button.h MBB.h Figure.h Node.h RTree.h test.h)

set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_DIR C:/SFML/lib/cmake/SFML)
find_package(SFML COMPONENTS system window graphics audio network REQUIRED)

include_directories(C:/SFML/include)

target_link_libraries(lines sfml-system sfml-window sfml-graphics sfml-audio)