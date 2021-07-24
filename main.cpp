#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "appvulkancore.h"

int main(){
    AppVulkanCore app(600, 800);
    try {
        app.run();
    }  catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
