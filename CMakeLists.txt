cmake_minimum_required(VERSION 3.14)

# General settings
set(CMAKE_CONFIGURATION_TYPES "Release;Debug;RelWithDebInfo")

set(CMAKE_CXX_STANDARD 14)

set(PROJ_NAME Experiments)
project(${PROJ_NAME})

file(GLOB_RECURSE EXPERIMENTS_HEADERS "Experiments/include/*.h")
file(GLOB_RECURSE EXPERIMENTS_SOURCES "Experiments/src/*.cpp")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Experiments/src" PREFIX "Experiments" FILES ${EXPERIMENTS_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Experiments/include" PREFIX "Experiments" FILES ${EXPERIMENTS_HEADERS})

add_executable(${PROJ_NAME} ${EXPERIMENTS_SOURCES} ${EXPERIMENTS_HEADERS})

target_include_directories(${PROJ_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Experiments/include")

if(MSVC)
    add_definitions(/MP)
    add_definitions(/D _CRT_SECURE_NO_WARNINGS)
endif()
