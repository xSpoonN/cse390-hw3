#include <iostream>
#include <fstream>
#include <cstring>
#include <filesystem>
#include <thread>
#include <condition_variable>
#include "../Common/AlgorithmRegistrar.h"
#include <dlfcn.h>
#include "./include/Simulator.h"

/* using AlgorithmPtr = std::unique_ptr<AlgorithmRegistrar>; */
using std::string;

int main(int argc, char** argv) {
    bool printouts = false;
    /* Default values */
    std::string house_path = "./";
    std::string algo_path = "./";
    int max_threads = 10;

    /* Parse Command Line Arguments */
    for (int i = 1; i < argc; i++) {
        if (std::strstr(argv[i], "-house_path=") == argv[i]) {
            house_path = std::string(argv[i]).substr(std::strlen("-house_path="));
            if (printouts) std::cerr << "Setting house path to " << house_path << std::endl;
        }
        else if (std::strstr(argv[i], "-algo_path=") == argv[i]) {
            algo_path = std::string(argv[i]).substr(std::strlen("-algo_path="));
            if (printouts) std::cerr << "Setting algo path to " << algo_path << std::endl;
        }
        else if (std::strstr(argv[i], "-num_threads=") == argv[i]) {
            max_threads = std::stoi(std::string(argv[i]).substr(std::strlen("-num_threads=")));
            if (max_threads <= 0) max_threads = 10;
            if (printouts) std::cerr << "Setting max threads to " << max_threads << std::endl;
        }
        else if (std::strstr(argv[i], "-verbose") == argv[i]) {
            printouts = true;
        }
    }

    /* Load Houses */
    std::filesystem::path housePath(house_path);
    std::vector<string> houses;
    for (const auto& entry : std::filesystem::directory_iterator(housePath)) {
        if (entry.is_regular_file() && (entry.path().extension() == ".house")) {
            // std::cerr << "Found a house" << std::endl;
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
                err_outfile.close();
                // std::cerr << "Error loading algorithm library: " << err << std::endl;
            } else if (AlgorithmRegistrar::getAlgorithmRegistrar().end() == prevSize) {
                std::ofstream err_outfile(error_out);
                err_outfile << "Error loading algorithm library: No Algorithm Registered!" << std::endl;
                err_outfile.close();
                // std::cerr << "Error loading algorithm library: No Algorithm Registered!" << std::endl;
            } else {
                // std::cerr << "Found an algo" << std::endl;
                libraries.push_back(algorithm_handle); 
            }
        }
    }

    /* Set up multithreading variables/mutexes */
    std::vector<std::thread> threads;
    std::mutex mut;
    std::mutex cerr_mut;
    std::condition_variable cv;
    int current_threads = 0;

    /* Run Simulation */
    for(const auto& algo: AlgorithmRegistrar::getAlgorithmRegistrar()) {
        for (const auto& house : houses) {
            /* Ensure that we haven't created more than the max # threads */
            std::unique_lock<std::mutex> lock(mut);
            while (current_threads >= max_threads) cv.wait(lock);
            ++current_threads;

            /* Create a thread and run the simulator */
            threads.emplace_back([=, &current_threads, &mut, &cv, &cerr_mut](){
                /* Print thread, algo, and house info to stderr */
                if (printouts) {
                    std::lock_guard<std::mutex> lock(cerr_mut);
                    std::cerr << "Thread [" << std::this_thread::get_id() << "] running algo [" << algo.name() << "] on house [" << house << ']' << std::endl;
                }
                /* Actually run the simulator */
                Simulator sim;
                sim.readHouseFile(house);
                std::unique_ptr<AbstractAlgorithm> algorithm = algo.create();
                sim.setAlgorithm(*algorithm);
                sim.run();
                /* When finished, decrement current_threads counter */
                { std::lock_guard<std::mutex> lock(mut); --current_threads; }
                /* Notify main thread */
                cv.notify_one();
            });
        }
    }

    /* Wait for all threads to finish */
    for (auto& t : threads) {
        t.join();
    }

    /* Free Libraries*/
    AlgorithmRegistrar::getAlgorithmRegistrar().clear();
    for (void *lib : libraries) dlclose(lib);

    return 0;
}
