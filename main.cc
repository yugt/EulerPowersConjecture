#include <iostream>
#include <numeric>
#include <atomic>
#include <limits>
#include <thread>
#include <vector>
#include <cmath>
#include <mutex>
#include <condition_variable>
#include <gmpxx.h>

using std::mutex;
using std::vector;
using std::thread;
using std::unique_lock;
using std::numeric_limits;

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
	vector<vector<size_t>> result;
	std::condition_variable cv;
	std::atomic<size_t> counter;
	std::atomic<bool> found;
	vector<thread> tasks;
	const size_t length;
	const size_t power;
	const size_t range;
	bool search_min;
	mutex m;

	// multithread faster than OpenMP parallel for
	void thread_parent(size_t up_bound){
		counter=0;
		found=false;
		tasks.reserve(up_bound-range);
		for(size_t i=range; i<up_bound; ++i){
			tasks.push_back(thread([this,i]{thread_child(i);}));
			// thread_child(i);
		}
		for(auto& n:tasks) n.join();
		unique_lock<mutex> lck(m);
		cv.wait(lck, [this]{return counter==tasks.size();});
	}

	void thread_child(size_t sum){
		vector<size_t> v(length,1);
		generate_simplex(v,sum,0);
		unique_lock<mutex> _(m);
		counter++;
		cv.notify_one();
	}

	void check_sum(const vector<size_t>& v){
		long double powersum=0.0;
		for(auto&n:v){
			powersum+=pow(n, power);
		}
		// not tested for large exponent (power>10)
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
		unique_lock<mutex> _(m);
		found=true;
		result.push_back({sum});
		result.back().insert(result.back().end(), v.begin(), v.end());
	}

	// single thread, recursion
	void generate_simplex(vector<size_t>& v, const size_t sum, size_t idx){
		if(search_min && found) return;
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
		length(l), power(p), range(std::max(r,l)) {
			if(power>10){
				std::cerr << "warning: unstable\n";
			}
			search_min = true;
	}

	// stop after finding minimum
	size_t configure(bool find_min){
		search_min = find_min;
		return thread::hardware_concurrency();
	}

	// called by MPI node
	void run(size_t width){
		thread_parent(range+width);
		for(auto&v : result){
			printf("%lu^%lu\t=\t", v[0], power);
			printf("%lu^%lu", v[1], power);
			for(auto it=v.begin()+2; it!=v.end();it++){
				printf("\t+\t%lu^%lu", *it, power);
			}
			printf("\n");
		}
	}
};

int main(int argc, char* argv[]){
	if(argc!=4){
		std::cerr << "input:\tlength\tpower\trange\n";
		return EXIT_FAILURE;
	}
	EulerSum instance(
		static_cast<size_t>(atoi(argv[1])),
		static_cast<size_t>(atoi(argv[2])),
		static_cast<size_t>(atoi(argv[3]))
		);
	instance.configure(true);
	instance.run(1024);
	return EXIT_SUCCESS;
}
