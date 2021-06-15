std = -std=c++17
libs = -lm -lgmp -pthread
flag = -Wall -Wextra -Wpedantic
exce = main


EulerSum: EulerSum.cc
	g++ $< -g -c $(flag) $(std) $(libs) -o $@.o


mpi_call: mpi_caller.cc EulerSum.o
	mpicxx $^ $(std) $(libs) $(flag) -o $(exce)

clean:
	rm $(exce) *.o
