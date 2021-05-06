cmake_minimum_required(VERSION 3.10)

project(voxel)

add_definitions(-DGLM_ENABLE_EXPERIMENTAL -DTRACY_ENABLE)

set(CMAKE_CXX_FLAGS "-O3")

add_executable(voxel voxel.cpp world.cpp redstone.cpp stb_image.cpp stb_include.cpp tracy/TracyClient.cpp)
target_compile_features(voxel PRIVATE cxx_std_17)

target_link_libraries(voxel ${CMAKE_DL_LIBS})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(voxel Threads::Threads)

find_package(OpenMP REQUIRED)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")

find_package(SDL2 REQUIRED)
target_link_libraries(voxel SDL2)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
target_link_libraries(voxel OpenGL::GL)

find_package(GLEW REQUIRED)
target_link_libraries(voxel GLEW::GLEW)
