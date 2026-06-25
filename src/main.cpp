#include "server.h"
#include <iostream>
#include <stdexcept>
#include <filesystem>

int main(int argc, char* argv[]) {
    int port        = 8080;
    size_t threads  = 4;
    std::string root = std::filesystem::absolute("public").string();

    if (argc >= 2) port    = std::stoi(argv[1]);
    if (argc >= 3) threads = std::stoul(argv[2]);
    if (argc >= 4) root    = argv[3];

    try {
        Server server(port, threads, root);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
