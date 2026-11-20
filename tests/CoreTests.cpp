#include "morpheus/MorpheusCore.h"

#include <cstdlib>
#include <iostream>

int main() {
    morpheus::MorpheusCore core;
    if (core.isInitialized()) {
        std::cerr << "A new core must not claim to be initialized.\n";
        return EXIT_FAILURE;
    }

    core.reset();
    if (core.isInitialized()) {
        std::cerr << "The research scaffold must not claim hardware emulation.\n";
        return EXIT_FAILURE;
    }

    std::cout << "Research scaffold sanity checks passed.\n";
    return EXIT_SUCCESS;
}
