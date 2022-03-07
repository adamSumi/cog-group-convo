cmake_minimum_required(VERSION 3.21)
project(cog_group_convo_cpp)
set(CMAKE_CXX_STANDARD 17)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_executable(${PROJECT_NAME} src/main.cpp)

find_package(SDL2 REQUIRED)
find_package(SDL2TTF REQUIRED)

target_link_libraries(${PROJECT_NAME} ${SDL2_LIBARIES} ${SDL2TTF_LIBRARY})