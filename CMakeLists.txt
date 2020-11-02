cmake_minimum_required(VERSION 3.14)

# General settings
set(CMAKE_CONFIGURATION_TYPES "Release;Debug;RelWithDebInfo")

set(CMAKE_CXX_STANDARD 17)

set(PROJ_NAME Experiments)
project(${PROJ_NAME})

# ImGui to a lib
file(GLOB_RECURSE FLECS_HEADERS "extern/flecs/include/*.h")
file(GLOB_RECURSE FLECS_SOURCES "extern/flecs/src/*.c")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/extern/flecs/include/flecs" PREFIX "Flecs" FILES ${FLECS_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/extern/flecs/src" PREFIX "Flecs" FILES ${FLECS_SOURCES})

add_library(Flecs ${FLECS_HEADERS} ${FLECS_SOURCES})

target_include_directories(Flecs PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/extern/flecs/include/flecs")
target_include_directories(Flecs PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/extern/flecs/include")

# Experiments
file(GLOB_RECURSE EXPERIMENTS_NATVIS "experiments/include/*.natvis")
file(GLOB_RECURSE EXPERIMENTS_HEADERS "experiments/include/*.h")
file(GLOB_RECURSE EXPERIMENTS_SOURCES "experiments/src/*.cpp")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/experiments/src" PREFIX "Experiments" FILES ${EXPERIMENTS_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/experiments/include" PREFIX "Experiments" FILES ${EXPERIMENTS_HEADERS})

add_executable(${PROJ_NAME} ${EXPERIMENTS_SOURCES} ${EXPERIMENTS_HEADERS} ${EXPERIMENTS_NATVIS})

target_include_directories(${PROJ_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/experiments/include")
target_include_directories(${PROJ_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/extern/stb/include")
target_link_libraries(${PROJ_NAME} PRIVATE Flecs)

if(MSVC)
    add_definitions(/MP)
    add_definitions(/D _CRT_SECURE_NO_WARNINGS)
endif()
