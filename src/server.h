#pragma once
#include "thread_pool.h"
#include <string>
#include <memory>

class Server {
public:
    Server(int port, size_t num_threads, const std::string& static_root);
    ~Server();

    void run();

private:
    void handle_client(int client_fd);

    int port;
    int server_fd;
    std::string static_root;
    std::unique_ptr<ThreadPool> pool;
};
