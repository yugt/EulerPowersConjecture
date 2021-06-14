#include <iostream>
#include <numeric>
#include <limits>
#include <thread>
#include <vector>
#include <cmath>
#include <mutex>
#include <gmpxx.h>

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
	const long double max_sum =
	static_cast<double>(numeric_limits<size_t>::max());
	size_t length;
	size_t power;
	size_t range;
	mutex m;

	// multithread faster than OpenMP parallel for
	void thread_parent(){
		vector<thread> v;
		v.reserve(range);
		for(size_t i=range; i<2*range; ++i){
			v.push_back(thread([this,i]{thread_child(i);}));
			// thread_child(i);
		}
		for(auto& n:v) n.join();
	}

	void thread_child(size_t sum){
		vector<size_t> v(length,1);
		generate_simplex(v,sum,0);
	}

	void check_sum(const vector<size_t>& v){
		long double powersum=0.0;
		for(auto&n:v){
			powersum+=pow(n, power);
		}
		long double sumroot = round(pow(powersum, 1.0/power));
		if(abs(powersum-pow(sumroot, power))<1e-3){
			check_sum(v, static_cast<size_t>(sumroot));
		}
	}

	void check_sum(const vector<size_t>&v, const size_t candidate){
		mpz_t ps, rs;
		mpz_init(rs);
		mpz_init_set_ui(ps, candidate);
		mpz_pow_ui(ps, ps, power);
		for(auto&n:v){
			mpz_set_ui(rs, n);
			mpz_pow_ui(rs, rs, power);
			mpz_sub(ps, ps, rs);
		}
		if(mpz_cmp_ui(ps, 0)==0){
			print_solution(v, candidate);
		}
	}

	void print_solution(const vector<size_t>& v, const size_t sum){
		m.lock(); // avoid mixing outputs from different threads
		printf("%lu^%lu\t=\t", sum, power);
		printf("%lu^%lu", v.front(), power);
		for(auto it=v.begin()+1; it!=v.end();it++){
			printf("\t+\t%lu^%lu", *it, power);
		}
		printf("\n");
		m.unlock();
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
				accumulate(v.begin(), v.begin()+idx+1, 0)<=
				sum-(idx+2==v.size() ? v[idx] : v.size()-idx);
				v[idx]++){
					generate_simplex(v, sum, idx+1);
			}
		}
	}

public:
	EulerSum(size_t l, size_t p, size_t r):
		length(l), power(p), range(r) {
			if(range<length){
				range=length;
			}
			if(power>10){
				cerr << "warning: unstable\n";
			}
			thread_parent();
	}
};

int main(int argc, char* argv[]){
	if(argc!=4){
		cerr << "input:\tlength\tpower\trange\n";
		return EXIT_FAILURE;
	}
	EulerSum _(
		static_cast<size_t>(atoi(argv[1])),
		static_cast<size_t>(atoi(argv[2])),
		static_cast<size_t>(atoi(argv[3]))
		);
	return EXIT_SUCCESS;
}
