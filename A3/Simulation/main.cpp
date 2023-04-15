#include <iostream>
// #include "Simulation/Simulator.h"
// #include "AlgorithmCommon/MyAlgorithm.h"
#include "../Common/AlgorithmRegistrar.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

using AlgorithmPtr = std::unique_ptr<AlgorithmRegistrar>;

// getting command line arguments for the house file
int main(int argc, char **argv)
{

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

    for(const auto& algo: AlgorithmRegistrar::getAlgorithmRegistrar()) {
        auto algorithm = algo.create();
        std::cout << algo.name() << ": " << static_cast<int>(algorithm->nextStep()) << std::endl;
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
