#include "App.hpp"
#include <iostream>

int main() {

    std::cout << "Silhouette starting...\n";

    App app;

    if (!app.init())
    {
        std::cerr << "Failed to initialise App.\n";
        return 1;
    }
    app.run();
    app.shutdown();

    std::cout << "Silhouette exiting.\n";
    return 0;
}