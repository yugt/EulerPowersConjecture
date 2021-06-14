compiler = g++
standard = -std=c++17
source = main.cc
libs = -lm -lgmp -pthread
exce = main

debug:
	$(compiler) -g $(standard) $(libs) $(source) -o $(exce)

release: $(exce)
	$(compiler) -O3 $(standard) $(libs) $(source) -o $(exce)

clean:
	rm $(exce)
