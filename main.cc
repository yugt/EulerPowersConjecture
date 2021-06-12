#include <iostream>
#include <cassert>
#include <numeric>
#include <ranges>
#include <vector>
#include <cmath>

using namespace std;

template<class num>
num gcd(num a, num b){
	return a==0 ? b : gcd(b%a, a);
}

template<class container>
auto gcd(container C){
	auto ans = C.front();
	for(auto& n : C){
		ans = gcd(ans, n);
		if(ans==1){return ans;}
	}
	return ans;
}

class EulerSum{
	size_t length;
	size_t power;
	size_t range;
	void thread_parent(){
		// serial implementation
		auto r=ranges::iota_view(range, 2*range);
		for_each(r.begin(), r.end(), thread_child);
	}

	void thread_child(size_t sum){
		vector<size_t> v(length,1);
		generate_simplex(v,sum,0);
	}

	void check_sum(const vector<size_t>& v){
		double powersum=0.0;
		for(auto&n:v){
			powersum+=pow(n, power);
		}
		double sumroot = round(pow(powersum, 1.0/power));
		if(abs(powersum-pow(sumroot, power))<1e-2){
			print_solution(v, sumroot);
		}
	}

	// solutions are sparse, no need for mutex
	void print_solution(const vector<size_t>& v, double sum){
		printf("%.0f^%lu\t=\t", sum, power);
		printf("%lu^%lu", v.front(), power);
		for(auto it=v.begin()+1; it!=v.end();it++){
			printf("\t+\t%lu^%lu", *it, power);
		}
		printf("\n");
	}

	// single thread, recursion
	void generate_simplex(vector<size_t>& v, const size_t sum, size_t idx){
		if(idx+1==v.size()){
			v.back()=sum-accumulate(v.begin(), v.begin()+idx, 0);
			if(gcd(v)==1){
				check_sum(v);
			}
		}else{
			for(v[idx]=(idx==0 ? 1: v[idx-1]);
			accumulate(v.begin(), v.begin()+idx+1, 0)<=sum-(idx+2==v.size() ? v[idx] : v.size()-idx);
			v[idx]++){
				generate_simplex(v, sum, idx+1);
			}
		}
	}

public:
	EulerSum(auto l, auto p, auto r):
		length(l), power(p), range(r) {
			if(range<length){
				cerr << endl;
				return;
			}
			thread_parent();
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