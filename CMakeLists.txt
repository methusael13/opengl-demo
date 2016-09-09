# CMake for OpenGL application
# @author: Methusael Murmu

set(PROJECT_NAME OpenGLDemo)

cmake_minimum_required(VERSION 2.8)
project(${PROJECT_NAME})

set(PROJECT_SRC src)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(LOCAL_LIB_DIR ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Enable C++11 standard and level 3 optimization
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3")

find_package(OpenGL)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

add_library(base_lib
    ${PROJECT_SRC}/base/gl3w.c
    ${PROJECT_SRC}/base/base_app.cpp
    ${PROJECT_SRC}/base/base_shader.cpp
    ${PROJECT_SRC}/base/base_util.cpp
    ${PROJECT_SRC}/base/3d/camera.cpp
    ${PROJECT_SRC}/base/text/text.cpp
)

# Static libraries
set(SOIL_LIB_FILE libSOIL.a)
add_library(soil_lib STATIC IMPORTED)
set_target_properties(soil_lib PROPERTIES
    IMPORTED_LOCATION ${LOCAL_LIB_DIR}/${SOIL_LIB_FILE})

# Shared libraries
set(ASSIMP_LIB_FILE libassimp.so)
add_library(assimp_lib SHARED IMPORTED)
set_target_properties(assimp_lib PROPERTIES
    IMPORTED_LOCATION ${LOCAL_LIB_DIR}/${ASSIMP_LIB_FILE})

set(FREETYPE_LIB_FILE libfreetype.so)
add_library(freetype_lib SHARED IMPORTED)
set_target_properties(freetype_lib PROPERTIES
    IMPORTED_LOCATION ${LOCAL_LIB_DIR}/${FREETYPE_LIB_FILE})

set(COMMON_LIBS freetype_lib soil_lib assimp_lib m base_lib dl z
    ${GLFW_LIBRARIES} ${OPENGL_LIBRARIES})
INCLUDE_DIRECTORIES(include ${GLFW_INCLUDE_DIRS})

set(EXES
    basic triag rect rect_rot fps_cam
    light_basic light_map light_cast
    model cube_map
    geom model_exp env_map
    planet
    shadow_map
    shadow_map_pl
)

foreach(EXE ${EXES})
    add_executable(${EXE} ${PROJECT_SRC}/${EXE}/${EXE}.cpp)
    target_link_libraries(${EXE} ${COMMON_LIBS})
endforeach(EXE)
