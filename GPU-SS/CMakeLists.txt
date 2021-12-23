cmake_minimum_required(VERSION 2.8)
project(GPU-SS)
set(CMAKE_CXX_STANDARD 11)
aux_source_directory(. SRC_LIST)
if(WIN32)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
    include_directories(${CMAKE_SOURCE_DIR}/includes)
    link_directories(${CMAKE_SOURCE_DIR}/lib)
    find_package(glfw3 REQUIRED)
    file(GLOB project_file glad.c main.cpp)
    add_executable(${PROJECT_NAME} ${SRC_LIST} GPUSupervoxel.cpp GPUSupervoxel.h Texture.h Texture.cpp)
    target_link_libraries(${PROJECT_NAME} ${GLFW3_LIBRARY})
elseif(UNIX AND NOT APPLE)
    find_package(glfw3 REQUIRED)
    file(GLOB project_file glad.c main.cpp)
    add_executable(${PROJECT_NAME} ${SRC_LIST} GPUSupervoxel.cpp GPUSupervoxel.h Texture.h Texture.cpp)
    target_link_libraries(${PROJECT_NAME}
            -lglfw3
            -lGL
            -lm
            -lXrandr
            -lXi
            -lX11
            -lXxf86vm
            -lpthread
            -ldl
            )
endif(WIN32)