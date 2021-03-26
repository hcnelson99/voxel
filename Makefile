voxel: voxel.cpp
	g++ -o voxel voxel.cpp -g -std=c++11 -pthread -lSDL2 -lGLEW -lX11 -lGLU -lGL
