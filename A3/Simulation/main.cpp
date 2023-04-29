#include <iostream>
#include <cstring>
#include <filesystem>
#include "../Common/AlgorithmRegistrar.h"
#include <dlfcn.h>
// #include "Simulation/Simulator.h"
// #include "AlgorithmCommon/MyAlgorithm.h"

using AlgorithmPtr = std::unique_ptr<AlgorithmRegistrar>;
using std::string;

// getting command line arguments for the house file
int main(int argc, char** argv) {

    // TODO: Create the simulator, move the code loading the algorithms into simulator class, 
    //       make sure you load all algorithms in the proper folder and not only two.

    // Load algorithm library


    std::string house_path = "./";
    std::string algo_path = "./";

    /* Parse Command Line Arguments */
    for (int i = 1; i < argc; i++) {
        if (std::strstr(argv[i], "-house_path=") == argv[i]) {
            house_path = std::string(argv[i]).substr(std::strlen("-house_path="));
        }
        else if (std::strstr(argv[i], "-algo_path=") == argv[i]) {
            algo_path = std::string(argv[i]).substr(std::strlen("-algo_path="));
        }
    }

    /* Load Houses*/
    std::filesystem::path housePath(house_path);
    std::vector<string> houses;
    for (const auto& entry : std::filesystem::directory_iterator(housePath)) {
        if (entry.is_regular_file() && (entry.path().extension() == ".house")) {
            houses.push_back(entry.path().string().c_str());
        }
    }

    /* Load Algorithms .dll/.so */
    std::filesystem::path algoPath(algo_path);
    std::vector<void*> libraries;
    for (const auto& entry : std::filesystem::directory_iterator(algoPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".so") {
            void* algorithm_handle = dlopen(entry.path().string().c_str(), RTLD_LAZY);
            if (!algorithm_handle) {
                std::cerr << "Error loading algorithm library: " << dlerror() << std::endl;
                return 1;
            }
            libraries.push_back(algorithm_handle); // This line might not be needed, gotta research how these so/dlls work
        }
    }

    for (void* lib : libraries) { // This block might not be needed, gotta research how these so/dlls work
        
    }

    /* Free .dll/.so */
    for (void* lib : libraries) {
        dlclose(lib);
    }

    for(const auto& algo: AlgorithmRegistrar::getAlgorithmRegistrar()) {
        auto algorithm = algo.create();
        /* delete */
        std::cout << algo.name() << ": " << static_cast<int>(algorithm->nextStep()) << std::endl;
        /* ****** */
        /* make new simulator instance and run the algo on input. */
    }

    AlgorithmRegistrar::getAlgorithmRegistrar().clear();
    // dlclose(algorithm_handle1);
    // dlclose(algorithm_handle2);
    return 0;
}
