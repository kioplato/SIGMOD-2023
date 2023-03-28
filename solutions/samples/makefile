# CC = clang++
# OPTION = -I./ -DIN_PARALLEL -Xpreprocessor -fopenmp -lomp
# LFLAGS = -std=c++11 -O3 $(OPTION)

CC = g++
OPTION = -I./ -DIN_PARALLEL -fopenmp
LFLAGS = -std=c++11 -O3 $(OPTION)

all: baseline

baseline : knn-construction.cc
	$(CC) $(LFLAGS) knn-construction.cc -o $@

clean :
	rm -f baseline
