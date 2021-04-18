OPTFLAGS=-march=native -O2
LDFLAGS=-pthread -lSDL2 -lGLEW -lX11 -lGLU -lGL -ldl 
CXXFLAGS=-g $(OPTFLAGS) -std=c++17 $(LDFLAGS) -DGLM_ENABLE_EXPERIMENTAL -DTRACY_ENABLE

SOURCES=voxel.cpp world.cpp redstone.cpp
HEADERS=world.h log.h repl.h ray.h

voxel: $(SOURCES) $(HEADERS) TracyClient.o
	g++ -o voxel $(SOURCES) TracyClient.o $(CXXFLAGS)

TracyClient.o: tracy/TracyClient.cpp
	g++ -c $^ $(CXXFLAGS)

editor: editor.cpp world.cpp
	g++ -o editor editor.cpp world.cpp $(CXXFLAGS)

.PHONY: clean
clean:
	rm -f TracyClient.o voxel editor
