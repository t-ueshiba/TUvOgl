cmake_minimum_required(VERSION 3.5)
project(ogltest)

file(GLOB sources *.cc)
add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} TUvOgl ${GTK2_LIBRARIES})

