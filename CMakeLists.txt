cmake_minimum_required(VERSION 2.4)
project(TUOgl++)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif()

enable_language(CXX)
set(CMAKE_CXX_STANDARD		14)
set(CMAKE_CXX_STANDARD_REQUIRED	ON)
set(CMAKE_CXX_EXTENSIONS	OFF)
set(CMAKE_CXX_FLAGS_RELEASE	"-Wall -O3")
set(CMAKE_CXX_FLAGS_DEBUG	"-g")
set(CMAKE_BUILD_TYPE		Release)
add_definitions(-DNDEBUG)

find_package(X11 REQUIRED)
find_package(OpenGL REQUIRED)
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_INSTALL_PREFIX}/include
    ${OPENGL_INCLUDE_DIRS}
    ${X11_INCLUDE_DIRS})

file(GLOB sources *.cc)
add_library(${PROJECT_NAME} SHARED ${sources})
target_link_libraries(
    ${PROJECT_NAME}
    TUv++
    ${OPENGL_gl_LIBRARY})

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)

add_subdirectory(TU)
