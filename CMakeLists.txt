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
find_package(SDL2_image REQUIRED)

include_directories(include)
add_executable(${PROJECT_NAME} src/main.cpp src/captions.cpp src/experiment_setup.cpp src/orientation.cpp src/presentation_methods.cpp include/experiment_setup.hpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})

# Add FlatBuffers directly to our build. This defines the `flatbuffers` target.
set(FLATBUFFERS_SRC_DIR libs/flatbuffers)
add_subdirectory(${FLATBUFFERS_SRC_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/flatbuffers-build
        EXCLUDE_FROM_ALL)

include_directories(${LIBVLC_INCLUDE_DIR} ${SDL2_IMAGE_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES} nlohmann_json::nlohmann_json ${SDL2TTF_LIBRARY} ${LIBVLC_LIBRARY} flatbuffers ${SDL2_IMAGE_LIBRARIES})


file(COPY resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})