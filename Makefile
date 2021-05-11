CXXFLAGS=-g -O3 -std=c++17 -pthread -lX11  -fopenmp -DGLM_ENABLE_EXPERIMENTAL

SOURCES=world.cpp redstone.cpp
HEADERS=world.h log.h repl.h ray.h config.h

.PHONY: tests
tests: tests/blocks tests/basic_redstone_propagation tests/basic_not_gate tests/basic_delay_gate tests/chained_not_gate tests/chained_delay_gate tests/blinker tests/and_gate tests/order_of_not_gates tests/loops tests/diode tests/switch tests/display tests/bluestone tests/loops_bluestone

tests/%: tests/%.cpp $(SOURCES)
	@echo "Running test: $@"
	@g++ -o $@ $(SOURCES) $< $(CXXFLAGS)
	@bash -c "timeout 5 $@ || printf \"\nTest failed: $@\n\n\""
	@rm $@

editor: editor.cpp $(SOURCES)
	g++ -o editor editor.cpp $(SOURCES) $(CXXFLAGS)

benchmark: benchmark.cpp $(SOURCES)
	g++ -static -o benchmark benchmark.cpp $(SOURCES) $(CXXFLAGS) $(CFLAGS) -DREDSTONE_BENCHMARK_ONLY

benchmark_worlds/%:
	@scripts/make_benchmark.sh "$@.world"
