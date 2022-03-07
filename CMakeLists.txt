cmake_minimum_required(VERSION 3.21)
project(cog_group_convo_cpp)
include(FetchContent)

FetchContent_Declare(json
        URL https://github.com/nlohmann/json/releases/download/v3.10.5/json.tar.xz
        )
FetchContent_GetProperties(json)
if (NOT json_POPULATED)
    FetchContent_Populate(json)
    add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()
set(CMAKE_CXX_STANDARD 20)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)
set(SOURCES
        src/main.cpp
        )
add_executable(${PROJECT_NAME} ${SOURCES})

include_directories(cog-flatbuffer-definitions)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(Threads)
find_package(Freetype REQUIRED)


# Add FlatBuffers directly to our build. This defines the `flatbuffers` target.
set(FLATBUFFERS_SRC_DIR ${LIB_DIR}/flatbuffers)
add_subdirectory(${FLATBUFFERS_SRC_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/flatbuffers-build
        EXCLUDE_FROM_ALL)

target_link_libraries(${PROJECT_NAME} PRIVATE flatbuffers Threads::Threads OpenGL::GL ${GLUT_LIBRARIES} nlohmann_json::nlohmann_json ${FREETYPE_LIBRARIES})
file(COPY ${SRC_DIR}/captions DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${SRC_DIR}
        ${FREETYPE_INCLUDE_DIRS}
        )