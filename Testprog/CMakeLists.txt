cmake_minimum_required(VERSION 2.4)
project(ogltest)

file(GLOB sources *.cc)
add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} TUOgl++ ${GTK2_LIBRARIES})

