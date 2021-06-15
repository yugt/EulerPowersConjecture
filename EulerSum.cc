#include <algorithm>
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
	vector<vector<size_t>> result;
	std::condition_variable cv;
	std::atomic<size_t> counter;
	std::atomic<bool> found;
	vector<thread> tasks;
	const size_t length;
	const size_t power;
	bool search_min;
	mutex m;

	// multithread faster than OpenMP parallel for
	void thread_parent(size_t begin, size_t end){
		counter=0;
		found=false;
		tasks.reserve(end-begin);
		for(size_t i=begin; i<end; ++i){
			tasks.push_back(thread([this,i]{thread_child(i);}));
		}
		{// parallel begin
			for(auto& n:tasks) n.join();
			unique_lock<mutex> lck(m);
			// wait until all tread_children finish
			cv.wait(lck, [this]{return counter==tasks.size();});
		}// parallel end
		if(result.size()>1){
			std::sort(result.begin(), result.end(),
				[](const vector<size_t>& a, const vector<size_t>& b)
					{return a.front() < b.front();});
			if(search_min) result.resize(1);
		}
	}

	void thread_child(size_t sum){
		vector<size_t> v(length, 1);
		generate_simplex(v, sum, 0);
		unique_lock<mutex> _(m);
		counter++;
		if(counter==tasks.size())
			cv.notify_one();
	}

	void check_sum(const vector<size_t>& v){
		long double power_sum=0.0;
		for(auto&n:v) power_sum+=pow(n, power);	
		// not tested for large exponent (power>10)
		long double sum_root = round(pow(power_sum, 1.0L/power));
		if(abs(power_sum-pow(sum_root, power))<1e-3){
			check_sum(v, static_cast<size_t>(sum_root));
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
			append_solution(v, candidate);
		}
	}

	void append_solution(const vector<size_t>& v, const size_t sum){
		unique_lock<mutex> _(m);
		found=true;
		result.push_back({sum});
		result.back().insert(result.back().end(), v.begin(), v.end());
	}

	// single thread, recursion, iterated for loops
	void generate_simplex(vector<size_t>& v, const size_t sum, size_t idx){
		if(search_min && found) return;
		if(idx+1==v.size()){
			v.back()=sum-accumulate(v.begin(), v.begin()+idx, 0);
			if(gcd(v)==1) check_sum(v);
		}else{
			for(v[idx]=(idx==0 ? 1: v[idx-1]);
				static_cast<size_t>(accumulate(v.begin(), v.begin()+idx+1, 0))<=
					sum-(idx+2==v.size() ? v[idx] : v.size()-idx);
				v[idx]++){
					generate_simplex(v, sum, idx+1);
			}
		}
	}

public:
	EulerSum(size_t l, size_t p) : length(l), power(p) {
	       	search_min = true;
	       	/*
	       	search result is not guaranteed to be the minimum:
	       	eg: length=2, power=2, range=[7..17]
	       	thread1: thread_child( 7) -> 5^2 = 3^2 + 4^2
	       	thread2: thread_child(17) -> 13^3 = 5^2 + 12^2
	       	but thread2 may return earlier than thread1
	       	*/
	       	// not tested for p>10
	}

	// stop after finding minimum
	size_t configure(bool find_min){
		search_min = find_min;
		return thread::hardware_concurrency();
	}

	// called by MPI node
	vector<vector<size_t>>& run(size_t begin, size_t width){
		begin = begin < length ? length : begin;
		thread_parent(begin, begin+width);
		return result;
	}
};
