cmake_minimum_required(VERSION 3.21)
project(cog_group_convo_cpp)
include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(SDL2 REQUIRED SDL2)
find_package(Threads)
find_package(Freetype REQUIRED)
find_package(LIBVLC REQUIRED)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.10.5/json.tar.xz)
FetchContent_MakeAvailable(json)

include_directories(include)
include_directories(${LIBVLC_INCLUDE_DIR})

set(FLATBUFFERS_SRC_DIR libs/flatbuffers)
add_subdirectory(${FLATBUFFERS_SRC_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/flatbuffers-build
        EXCLUDE_FROM_ALL)

#add_executable(${PROJECT_NAME} src/main.cpp include/orientation.hpp include/captions.hpp src/vlc.cpp)
add_executable(${PROJECT_NAME} src/main.cpp include/orientation.hpp include/captions.hpp)


target_link_libraries(${PROJECT_NAME} PRIVATE SDL2 Threads::Threads nlohmann_json::nlohmann_json SDL2_ttf flatbuffers ${FREETYPE_LIBRARIES} ${LIBVLC_LIBRARY})

file(COPY resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})