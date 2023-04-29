#include <iostream>
#include <cstring>
#include <filesystem>
#include "../Common/AlgorithmRegistrar.h"
#include <dlfcn.h>
#include "./include/Simulator.h"

/* using AlgorithmPtr = std::unique_ptr<AlgorithmRegistrar>; */
using std::string;

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
            void* algorithm_handle = dlopen(entry.path().string().c_str(), RTLD_LAZY);
            if (!algorithm_handle) {
                std::cerr << "Error loading algorithm library: " << dlerror() << std::endl;
                return 1;
            }
            std::cerr << "Found an algo" << std::endl;
            libraries.push_back(algorithm_handle);
        }
    }

    /* Run Simulation */
    for(const auto& algo: AlgorithmRegistrar::getAlgorithmRegistrar()) {
        for (const auto& house : houses) {
            std::cout << "Running algo " << algo.name() << " on house " << house << std::endl;
            Simulator sim;
            sim.readHouseFile(house);
            std::unique_ptr<AbstractAlgorithm> algorithm = algo.create();
            sim.setAlgorithm(*algorithm);
            std::string out_file = house.substr(0, house.find_last_of(".")) + "-" + algo.name() + ".txt";
            sim.run(out_file);
        }
    }

    /* Free Libraries*/
    AlgorithmRegistrar::getAlgorithmRegistrar().clear();
    for (void *lib : libraries) dlclose(lib);

    return 0;
}
