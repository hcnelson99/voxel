CXXFLAGS=-g -std=c++17 -pthread -lSDL2 -lGLEW -lX11 -lGLU -lGL -DGLM_ENABLE_EXPERIMENTAL

SOURCES=voxel.cpp world.cpp
HEADERS=world.h log.h repl.h ray.h

voxel: $(SOURCES) $(HEADERS)
	g++ -o voxel $(SOURCES) $(CXXFLAGS)

editor: editor.cpp world.cpp
	g++ -o editor editor.cpp world.cpp $(CXXFLAGS)
