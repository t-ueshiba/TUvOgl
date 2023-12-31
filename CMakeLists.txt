cmake_minimum_required(VERSION 3.5)
project(TUvOgl)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif()

enable_language(CXX)
set(CMAKE_CXX_STANDARD		17)
set(CMAKE_CXX_STANDARD_REQUIRED	ON)
set(CMAKE_CXX_EXTENSIONS	OFF)
set(CMAKE_CXX_FLAGS_RELEASE	"-Wall -O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG	"-g")

if(APPLE)
  set(MACOSX_RPATH TRUE)
endif()


find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)
find_package(Boost REQUIRED)
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_INSTALL_PREFIX}/include
    ${OPENGL_INCLUDE_DIRS}
    ${X11_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS})

link_directories(${CMAKE_INSTALL_PREFIX}/lib)

file(GLOB sources *.cc)
add_library(${PROJECT_NAME} SHARED ${sources})
target_link_libraries(${PROJECT_NAME} TUv GLU GL)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)

add_subdirectory(TU)
add_subdirectory(Testprog)
