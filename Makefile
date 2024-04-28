.PHONY: all
all: run-bench

bench: bench.cpp iterator_sorting.h
	g++ -O2 -std=c++20 bench.cpp -o bench

.PHONY: run-bench
run-bench: bench
	./bench
