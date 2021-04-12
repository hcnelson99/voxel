CXXFLAGS=-g -std=c++17 -pthread -lSDL2 -lGLEW -lX11 -lGLU -lGL -DGLM_ENABLE_EXPERIMENTAL

voxel: voxel.cpp world.cpp world.h log.h repl.h
	g++ -o voxel world.cpp voxel.cpp $(CXXFLAGS)
