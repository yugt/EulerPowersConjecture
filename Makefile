compiler = g++
standard = -std=c++17
source = main.cc
libs = -lm -lgmp -pthread
exce = main

debug:
	$(compiler) $(source) -g $(standard) $(libs) -o $(exce)

release: $(exce)
	$(compiler) $(source) -O3 $(standard) $(libs) -o $(exce)

clean:
	rm $(exce)
