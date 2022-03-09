cmake_minimum_required(VERSION 3.21)

include(FetchContent)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.10.5/json.tar.xz)
FetchContent_MakeAvailable(json)

project(cog_group_convo_cpp)
find_package(LIBVLC REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2TTF)

include_directories(include)
add_executable(${PROJECT_NAME} src/main.cpp include/AppContext.hpp)

# Add FlatBuffers directly to our build. This defines the `flatbuffers` target.
set(FLATBUFFERS_SRC_DIR libs/flatbuffers)
add_subdirectory(${FLATBUFFERS_SRC_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/flatbuffers-build
        EXCLUDE_FROM_ALL)

include_directories(${LIBVLC_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2 nlohmann_json::nlohmann_json ${SDL2TTF_LIBRARY} ${LIBVLC_LIBRARY} flatbuffers)


file(COPY resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})