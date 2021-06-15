#include "EulerSum.cc"
#include <iostream>
#include <mpi.h>

void print_soln(std::vector<vector<size_t>>& soln, size_t p){
	for(auto&v : soln){
		printf("%lu^%lu\t=\t", v[0], p);
		printf("%lu^%lu", v[1], p);
		for(auto it=v.begin()+2; it!=v.end();it++){
			printf("\t+\t%lu^%lu", *it, p);
		}
		printf("\n");
	}
}


int main(int argc, char* argv[]){
	if(argc!=4){
		std::cerr << "input:\tlength\tpower\trange\n";
		return EXIT_FAILURE;
	}
	size_t s[3]={0,0,0};
	for(int i=1; i<argc; ++i){
		s[i-1]=static_cast<size_t>(atoi(argv[i]));
	}
	EulerSum instance(s[0],s[1],s[2]);
	instance.configure(false);
	print_soln(instance.run(16), s[1]);
	return EXIT_SUCCESS;
}
