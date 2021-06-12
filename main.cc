#include <iostream>
#include <cassert>
#include <vector>

using namespace std;

class EulerSum{
	size_t length;
	size_t power;
	size_t range;
	void thread_manager(){
		// serial
		for(auto s=range; s<2*range; ++s){
		}
	}
public:
	EulerSum(auto l, auto p, auto r):
		length(l), power(p), range(r) {
			if(range<length){
				cerr << endl;
				return;
			}
			thread_manager();
	}
};

int main(int argc, char* argv[]){
	assert(argc==4);
	EulerSum _(
		static_cast<size_t>(atoi(argv[1])),
		static_cast<size_t>(atoi(argv[2])),
		static_cast<size_t>(atoi(argv[3]))
		);
	return EXIT_SUCCESS;
}