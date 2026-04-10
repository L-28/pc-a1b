#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include <numeric>
#include <thread>

#include "helper.hpp"

using namespace std;

struct histogram {
	std::vector<int> data;

	histogram(int count) : data(count) { }

	void add(int i) {
		++data[i];
	}

	void set(int i, int v){
	    data[i] = v;
	}

    void set_add(int i, int v){
	    data[i] += v;
	}

	int& get(int i)	{
		return data[i];
	}

	void print_total(std::ostream& str) {
		str << "total:" << accumulate(data.begin(), data.end(), 0) << "\n";
	}

	void print_bins(std::ostream& str) {
		for (size_t i = 0; i < data.size(); ++i) str << i << ":" << data[i] << "\n";
	}
};

mutex mtx;
void worker(int sample_count, histogram& hist, int num_bins)
{
	generator gen(num_bins);
	vector<int> h(num_bins, 0);
	while (sample_count--) {
		int next = gen();
		++h[next];
	}

    std::lock_guard<std::mutex> lock(mtx);
    for(int it = 0; it < h.size(); it++){
        hist.set_add(it, h[it]);
    }
}

void compute(int num_threads, int sample_count, histogram& h, int num_bins){
    num_threads *= 4;
    // auto start = chrono::high_resolution_clock::now();
    vector<std::thread> threads;
    threads.reserve(num_threads);
    int scount = sample_count/num_threads;
    std::vector<std::vector<int>> datas(num_threads, std::vector<int>(num_bins, 0));
    for(int tid = 0; tid < num_threads; ++tid){
		threads.push_back(std::thread(worker, scount, std::ref(h), num_bins));
	}
	// auto setup = chrono::high_resolution_clock::now();
	for(auto& t : threads){
        t.join();
    }
    // auto working = chrono::high_resolution_clock::now();
    // for(int it = 0; it < datas.size(); it++){
    //     for(int ite = 0; ite < datas[it].size(); ite++){
    //         h.set_add(ite, datas[it][ite]);
    //     }
    // }
    // auto finish = chrono::high_resolution_clock::now();
    // cout << "\nSetup: " << setup-start << "\nWork: " << working-setup << "\nFinish: " << finish-working << endl;
	return;
}

int main(int argc, char **argv)
{
	int num_bins = 10;
	int sample_count = 30000000;

	int num_threads = std::thread::hardware_concurrency();
	int print_level = 3; // 0: exec info + histogram total, 1: + histogram bins, 2: +exec time, 3: +bin info
	parse_args(argc, argv, num_threads, num_bins, sample_count, print_level);

	histogram h(num_bins);

	auto t1 = chrono::high_resolution_clock::now();

	//worker(sample_count, h, num_bins);
	compute(num_threads, sample_count, h, num_bins);

	auto t2 = chrono::high_resolution_clock::now();

	if ( print_level >= 0 ) cout << "Bins: " << num_bins << ", sample size: " << sample_count << ", threads: " << num_threads << endl;
	if ( print_level >= 3 ) h.print_bins(cout);
	if ( print_level >= 1 ) h.print_total(cout);
	if ( print_level >= 2 || print_level == -1 ) cout << chrono::duration<double>(t2 - t1).count() << endl;
}

