#include <iostream>
#include <cstring>
#include <filesystem>
// #include "Simulation/Simulator.h"
// #include "AlgorithmCommon/MyAlgorithm.h"
#include "../Common/AlgorithmRegistrar.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

using AlgorithmPtr = std::unique_ptr<AlgorithmRegistrar>;
using std::string;

// getting command line arguments for the house file
int main(int argc, char **argv) {

    // TODO: Create the simulator, move the code loading the algorithms into simulator class, 
    //       make sure you load all algorithms in the proper folder and not only two.

    // Load algorithm library
#ifdef _WIN32
    HMODULE algorithm_handle1 = LoadLibraryA("Algorithm_1_123456789\\Algorithm_1_123456789.dll");
    if (!algorithm_handle1) {
        std::cerr << "Error loading algorithm library: " << GetLastError() << std::endl;
        return 1;
    }

    HMODULE algorithm_handle2 = LoadLibraryA("Algorithm_2_123456789\\Algorithm_2_123456789.dll");
    if (!algorithm_handle2) {
        std::cerr << "Error loading algorithm library: " << GetLastError() << std::endl;
        AlgorithmRegistrar::getAlgorithmRegistrar().clear();
        FreeLibrary(algorithm_handle1);
        return 1;
    }
#else
    algorithm_handle1 = dlopen("Algorithm_1_123456789/libAlgorithm_1_123456789.so", RTLD_LAZY);
    if (!algorithm_handle1) {
        std::cerr << "Error loading algorithm library: " << dlerror() << std::endl;
        return 1;
    }

    algorithm_handle2 = dlopen("Algorithm_2_123456789/libAlgorithm_2_123456789.so", RTLD_LAZY);
    if (!algorithm_handle2) {
        std::cerr << "Error loading algorithm library: " << dlerror() << std::endl;
        AlgorithmRegistrar::getAlgorithmRegistrar().clear();
        dlclose(algorithm_handle1);
        return 1;
    }
#endif
    std::string house_path = "";
    std::string algo_path = "";

    /* Parse Command Line Arguments */
    for (int i = 1; i < argc; i++) {
        if (std::strstr(argv[i], "-house_path=") == argv[i]) {
            house_path = std::string(argv[i]).substr(std::strlen("-house_path="));
        }
        else if (std::strstr(argv[i], "-algo_path=") == argv[i]) {
            algo_path = std::string(argv[i]).substr(std::strlen("-algo_path="));
        }
    }
    if (house_path == "" || algo_path == "") {
		std::cerr << "Error: Missing arguments" << std::endl;
		return 1;
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
        if (entry.is_regular_file() && (entry.path().extension() == ".dll" || entry.path().extension() == ".so")) {
#ifdef _WIN32
            HMODULE algorithm_handle = LoadLibraryA(entry.path().string().c_str());
            if (!algorithm_handle1) {
                std::cerr << "Error loading algorithm library: " << GetLastError() << std::endl;
                return 1;
            }
#else
            algorithm_handle = dlopen(entry.path().string().c_str(), RTLD_LAZY);
            if (!algorithm_handle1) {
                std::cerr << "Error loading algorithm library: " << dlerror() << std::endl;
                return 1;
            }
#endif
            libraries.push_back(algorithm_handle); // This line might not be needed, gotta research how these so/dlls work
        }
    }

    for (void* lib : libraries) { // This block might not be needed, gotta research how these so/dlls work
#ifdef _WIN32
        // Call function from Windows DLL
#else
        // Call function from Unix SO
#endif
    }

    /* Free .dll/.so */
    for (void* lib : libraries) {
#ifdef _WIN32
        FreeLibrary((HMODULE)lib);
#else
        dlclose(lib);
#endif
    }



    for(const auto& algo: AlgorithmRegistrar::getAlgorithmRegistrar()) {
        auto algorithm = algo.create();
        /* delete */
        std::cout << algo.name() << ": " << static_cast<int>(algorithm->nextStep()) << std::endl;
        /* ****** */
        /* make new simulator instance and run the algo on input. */
    }

    AlgorithmRegistrar::getAlgorithmRegistrar().clear();
#ifdef _WIN32
    FreeLibrary(algorithm_handle1);
    FreeLibrary(algorithm_handle2);
#else
    dlclose(algorithm_handle1);
    dlclose(algorithm_handle2);
#endif
    return 0;
}
