CXXFLAGS=-g -std=c++17 -pthread -lSDL2 -lGLEW -lX11 -lGLU -lGL -DGLM_ENABLE_EXPERIMENTAL

SOURCES=world.cpp redstone.cpp
HEADERS=world.h log.h repl.h ray.h

.PHONY: tests
tests: tests/basic_redstone_propagation tests/basic_not_gate tests/basic_delay_gate tests/chained_not_gate tests/chained_delay_gate tests/blinker tests/and_gate tests/order_of_not_gates

tests/%: tests/%.cpp $(SOURCES)
	@echo "Running test: $@"
	@g++ -o $@ $(SOURCES) $< $(CXXFLAGS)
	@bash -c "$@ || true"
	@rm $@

editor: editor.cpp $(SOURCES)
	g++ -o editor editor.cpp $(SOURCES) $(CXXFLAGS)

