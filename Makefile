CXXFLAGS=-g -std=c++17 -pthread -lSDL2 -lGLEW -lX11 -lGLU -lGL -fopenmp -DGLM_ENABLE_EXPERIMENTAL

SOURCES=world.cpp redstone.cpp
HEADERS=world.h log.h repl.h ray.h config.h

.PHONY: tests
tests: tests/blocks tests/basic_redstone_propagation tests/basic_not_gate tests/basic_delay_gate tests/chained_not_gate tests/chained_delay_gate tests/blinker tests/and_gate tests/order_of_not_gates tests/loops tests/diode tests/switch tests/display tests/bluestone tests/loops_bluestone

.PHONY: test
test:
	g++ -o test test.cpp $(SOURCES) $(CXXFLAGS) $(CFLAGS)

tests/%: tests/%.cpp $(SOURCES)
	@echo "Running test: $@"
	@g++ -o $@ $(SOURCES) $< $(CXXFLAGS)
	@bash -c "timeout 5 $@ || printf \"\nTest failed: $@\n\n\""
	@rm $@

editor: editor.cpp $(SOURCES)
	g++ -o editor editor.cpp $(SOURCES) $(CXXFLAGS)

benchmark: benchmark.cpp $(SOURCES)
	g++ -O3 -o benchmark benchmark.cpp $(SOURCES) $(CXXFLAGS) $(CFLAGS)

benchmark_worlds/%:
	@scripts/make_benchmark.sh "$@.world"
