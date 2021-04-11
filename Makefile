voxel: voxel.cpp world.h
	g++ -o voxel voxel.cpp -g -std=c++11 -pthread -lSDL2 -lGLEW -lX11 -lGLU -lGL -DGLM_ENABLE_EXPERIMENTAL
