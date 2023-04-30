#include <iostream>
#include <fstream>
#include <cstring>
#include <filesystem>
#include <thread>
#include <future>
#include "../Common/AlgorithmRegistrar.h"
#include <dlfcn.h>
#include "./include/Simulator.h"

/* using AlgorithmPtr = std::unique_ptr<AlgorithmRegistrar>; */
using std::string;

void run_algo(AbstractAlgorithm& algo, const string& house_path, std::promise<void> promise) {
    std::cout << "thread started" << std::endl;
    Simulator sim;
    sim.readHouseFile(house_path);
    sim.setAlgorithm(algo);
    promise.set_value();
    sim.run();
}

int main(int argc, char** argv) {
    std::string house_path = "./";
    std::string algo_path = "./";

    /* Parse Command Line Arguments */
    for (int i = 1; i < argc; i++) {
        if (std::strstr(argv[i], "-house_path=") == argv[i]) {
            std::cerr << "House path found" << std::endl;
            house_path = std::string(argv[i]).substr(std::strlen("-house_path="));
        }
        else if (std::strstr(argv[i], "-algo_path=") == argv[i]) {
            std::cerr << "Algo path found" << std::endl;
            algo_path = std::string(argv[i]).substr(std::strlen("-algo_path="));
        }
    }

    /* Load Houses */
    std::filesystem::path housePath(house_path);
    std::vector<string> houses;
    for (const auto& entry : std::filesystem::directory_iterator(housePath)) {
        if (entry.is_regular_file() && (entry.path().extension() == ".house")) {
            std::cerr << "Found a house" << std::endl;
            houses.push_back(entry.path().string().c_str());
        }
    }

    /* Load Algorithms */
    std::filesystem::path algoPath(algo_path);
    std::vector<void*> libraries;
    for (const auto& entry : std::filesystem::directory_iterator(algoPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".so") {
            const auto& prevSize = AlgorithmRegistrar::getAlgorithmRegistrar().end();
            void* algorithm_handle = dlopen(entry.path().string().c_str(), RTLD_LAZY);
            std::string error_out = entry.path().string().substr(0, entry.path().string().find_last_of(".")) + ".error";
            std::size_t lastSlash = error_out.find_last_of("/\\"); /* Removes the preceding path from algo.error */
            if (lastSlash != std::string::npos) error_out = error_out.substr(lastSlash + 1);
            if (!algorithm_handle) {
                std::ofstream err_outfile(error_out); 
                string err = dlerror();
                err_outfile << "Error loading algorithm library: " << err << std::endl;
                std::cerr << "Error loading algorithm library: " << err << std::endl;
            } else if (AlgorithmRegistrar::getAlgorithmRegistrar().end() == prevSize) {
                std::ofstream err_outfile(error_out);
                err_outfile << "Error loading algorithm library: No Algorithm Registered!" << std::endl;
                std::cerr << "Error loading algorithm library: No Algorithm Registered!" << std::endl;
            } else {
                std::cerr << "Found an algo" << std::endl;
                libraries.push_back(algorithm_handle); 
            }
        }
    }

    // Create a vector for storing threads
    std::vector<std::thread> threads;

    /* Run Simulation */
    for(const auto& algo : AlgorithmRegistrar::getAlgorithmRegistrar()) {
        for (const auto& house_path : houses) {
            std::unique_ptr<AbstractAlgorithm> algorithm = algo.create();
            std::promise<void> promise;
            std::future<void> future = promise.get_future();
            std::thread thr(run_algo, std::ref(*algorithm), house_path, std::move(promise));
            std::cout << "Thread " << thr.get_id() << " running algo " << algo.name() << " on house " << house_path << std::endl;
            future.wait();
            threads.push_back(std::move(thr));
        }
    }

    std::cout << "Waiting for all threads to finish running" << std::endl;

    /* Wait for all threads to finish running */
    for (auto& thr : threads) {
        thr.join();
    }

    /* Free Libraries*/
    AlgorithmRegistrar::getAlgorithmRegistrar().clear();
    for (void *lib : libraries) dlclose(lib);

    return 0;
}
