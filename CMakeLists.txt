cmake_minimum_required(VERSION 3.10)

project(voxel)

add_definitions(-DGLM_ENABLE_EXPERIMENTAL -DTRACY_ENABLE)

add_executable(voxel voxel.cpp world.cpp redstone.cpp tracy/TracyClient.cpp)
target_compile_features(voxel PRIVATE cxx_std_17)

target_link_libraries(voxel ${CMAKE_DL_LIBS})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(voxel Threads::Threads)

find_package(SDL2 REQUIRED)
target_link_libraries(voxel SDL2)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
target_link_libraries(voxel OpenGL::GL)

find_package(GLEW REQUIRED)
target_link_libraries(voxel GLEW::GLEW)
