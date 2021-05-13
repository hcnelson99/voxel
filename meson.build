project('voxel', 'cpp', default_options : ['cpp_std=c++17', 'b_ndebug=if-release'])

openmp = dependency('openmp')
glew = dependency('glew')
sdl2 = dependency('sdl2')
glm = dependency('glm')
cc = meson.get_compiler('cpp')
dl = cc.find_library('dl')

srcs = ['voxel.cpp', 'world.cpp', 'redstone.cpp', 'stb_image.cpp', 'stb_include.cpp', 'tracy/TracyClient.cpp']
executable('voxel', srcs, dependencies : [openmp, glew, sdl2, glm, dl], cpp_args : '-DTRACY_ENABLE -DGLM_ENABLE_EXPERIMENTAL')
