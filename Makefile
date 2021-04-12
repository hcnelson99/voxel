CXXFLAGS=-g -std=c++14 -pthread -lSDL2 -lGLEW -lX11 -lGLU -lGL -DGLM_ENABLE_EXPERIMENTAL

voxel: voxel.cpp world.cpp world.h
	g++ -o voxel world.cpp voxel.cpp $(CXXFLAGS)
