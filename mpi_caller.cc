#include "EulerSum.cc"
#include <iostream>
#include <ctime>
#include <mpi.h>

using std::vector;

void print_soln(vector<vector<size_t>>& soln, size_t p){
	for(auto&v : soln){
		printf("%lu^%lu\t=\t", v[0], p);
		printf("%lu^%lu", v[1], p);
		for(auto it=v.begin()+2; it!=v.end();it++){
			printf("\t+\t%lu^%lu", *it, p);
		}
		printf("\n");
	}
}

auto cmd_args(int argc, char* argv[]){
	vector<size_t> ans(4, 0);
	for(size_t i=1; i<ans.size(); ++i){
		ans[i-1]=static_cast<size_t>(atoi(argv[i]));
	}
	ans[2] = ans[2]==0 ? 1 : ans[2];
	ans[3] = argc==4 ? 2*ans[2] :
		static_cast<size_t>(atoi(argv[4]));
	return ans;
}

void time_stamp(const size_t time_size){
	char* buffer = (char*)malloc(time_size);
	std::time_t now = std::time(nullptr);
	const struct std::tm *ptr = std::localtime(&now);
	std::strftime(buffer, time_size,
		"%H:%M:%S %b %d %Y %a\t%U %j", ptr);
	fprintf(stderr, "%s\n", buffer);
	free(buffer);
}

int finish_all(vector<size_t>& node, vector<int> tag){
	int Ierr = 0;
	size_t range[2] = {0,0};
	for(size_t rank=1; rank<node.size(); rank++){
		Ierr = MPI_Send(range, 2, MPI_UNSIGNED_LONG, rank,
						++tag[rank], MPI_COMM_WORLD);
	}
	return Ierr;
}

size_t child_node_task_init(const vector<size_t>& num_thread){
	vector<size_t> range(num_thread.size());
	partial_sum(num_thread.begin(), num_thread.end(), range.begin());
	// fprintf(stderr, "child node init\n");
	for(size_t i=1; i<range.size(); ++i){
		size_t r[2] = {range[i-1], range[i]};
		MPI_Send(r, 2, MPI_UNSIGNED_LONG, i, 1, MPI_COMM_WORLD);
		// fprintf(stderr, "new task to %lu [%lu,%lu) with tag %d\n",
		// 		i, r[0], r[1], 1);
	}
	return range.back();
}

// result collecting & parsing
int parent_node(const size_t procs, const vector<size_t>& spec){
	int Ierr = 0;
	MPI_Status status;
	vector<size_t> thread_count(procs, 0);
	for(size_t i=1;i<procs; ++i){
		Ierr = MPI_Recv(&thread_count.at(i), 1, MPI_UNSIGNED_LONG,
						i, 0, MPI_COMM_WORLD, &status);
	}
	thread_count.front() = spec[2];
	size_t range = child_node_task_init(thread_count);
	vector<int> tag(procs, 1);
	while(range<spec[3]){
		// assign new tasks to returning nodes
		Ierr = MPI_Recv(&thread_count.at(0), 1, MPI_UNSIGNED_LONG,
				MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		tag[status.MPI_SOURCE] = status.MPI_TAG;
		if(thread_count.front()==0) break;  // some child found soln
		size_t task[2] = {range, range+thread_count.front()};
		Ierr = MPI_Send(&task, 2, MPI_UNSIGNED_LONG, status.MPI_SOURCE,
					++tag[status.MPI_SOURCE], MPI_COMM_WORLD);
		// fprintf(stderr, "new task to %d [%lu,%lu) with tag %d\n",
		// 		status.MPI_SOURCE, task[0], task[1], tag[status.MPI_SOURCE]);
		range+=thread_count.front();
	}
	Ierr = finish_all(thread_count, tag);
	return Ierr;
}


// computing
int child_node(int rank, const vector<size_t>& spec){
	int Ierr = 0;
	MPI_Status status;
	EulerSum instance(spec[0], spec[1]);
	size_t range[2] = {0,0};
	size_t thread_count = instance.configure(true);
	Ierr = MPI_Send(&thread_count, 1, MPI_UNSIGNED_LONG,
					0, 0, MPI_COMM_WORLD);
	Ierr = MPI_Recv(&range, 2, MPI_UNSIGNED_LONG, 0,
					1, MPI_COMM_WORLD, &status);
	while(range[0]!=0 && range[1]!=0){
		// auto& result = instance.run(range[0], range[1]-range[0]);
		// if(result.empty()){
			fprintf(stderr, "node %d no soln [%6lu,%6lu)\n",
					rank, range[0], range[1]);
		// }else{
		// 	fprintf(stdout, "node %d found in [%lu,%lu)\n",
		// 			rank, range[0], range[1]);
		// 	print_soln(result, spec[1]);
		// 	thread_count = 0;
		// }
		Ierr = MPI_Send(&thread_count, 1, MPI_UNSIGNED_LONG,
						0, status.MPI_TAG, MPI_COMM_WORLD);
		Ierr = MPI_Recv(&range, 2, MPI_UNSIGNED_LONG, 0,
						status.MPI_TAG+1, MPI_COMM_WORLD, &status);
	}
	return Ierr;
}

int main(int argc, char* argv[]){
	if(argc<4 || argc>5){
		fprintf(stderr, "input:\tlength\tpower\trange\n");
		return EXIT_FAILURE;
	}
	const auto& s = cmd_args(argc, argv);
	int Ierr = MPI_Init(nullptr, nullptr);
	if(Ierr!=0){
		fprintf(stderr, "MPI failed to init\n");
		exit(EXIT_FAILURE);
	}
	int num_procs, rank = 0;
	Ierr = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
	Ierr = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	if(rank==0){
		Ierr = parent_node(num_procs, s);
	}else{
		Ierr = child_node(rank, s);
	}
	MPI_Finalize();
	// print_soln(instance.run(16,8), s[1]);
	return EXIT_SUCCESS;
}
