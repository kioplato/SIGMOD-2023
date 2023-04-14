# CC = clang++
# OPTION = -I./ -DIN_PARALLEL -Xpreprocessor -fopenmp -lomp
# LFLAGS = -std=c++11 -O3 $(OPTION)

CC = g++
OPTION = -I./ -DIN_PARALLEL -fopenmp
LFLAGS = -std=c++11 -O3 $(OPTION) -march=native -mtune=native

all: nnj

nnj : src/main.cpp src/heap.hpp
	$(CC) $(LFLAGS) src/main.cpp -o $@

clean :
	rm -f nnj
