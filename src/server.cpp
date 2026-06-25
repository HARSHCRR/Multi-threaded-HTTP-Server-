#include "server.h"
#include "http_parser.h"
#include "http_response.h"
#include "file_handler.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <sstream>

Server::Server(int port, size_t num_threads, const std::string& static_root)
    : port(port), server_fd(-1), static_root(static_root),
      pool(std::make_unique<ThreadPool>(num_threads)) {

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) throw std::runtime_error("socket() failed");

    // Allow immediate reuse of the port after restart
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("bind() failed");

    if (listen(server_fd, 128) < 0)
        throw std::runtime_error("listen() failed");
}

Server::~Server() {
    if (server_fd >= 0) close(server_fd);
}

void Server::run() {
    std::cout << "Server listening on port " << port << "\n";
    std::cout << "Serving files from: " << static_root << "\n";

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            std::cerr << "accept() failed\n";
            continue;
        }

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
        std::cout << "Client connected: " << ip << "\n";

        pool->enqueue([this, client_fd]() {
            handle_client(client_fd);
        });
    }
}

void Server::handle_client(int client_fd) {
    // Read the full request (stop after headers + body)
    std::string raw;
    char buf[4096];

    while (true) {
        ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
        if (n <= 0) break;
        raw.append(buf, n);

        // Check if we have the full headers
        auto header_end = raw.find("\r\n\r\n");
        if (header_end == std::string::npos) continue;

        // Parse to know if there's a body
        HttpRequest req = parse_request(raw);
        auto it = req.headers.find("content-length");
        if (it == req.headers.end()) break; // no body expected

        size_t content_len = std::stoul(it->second);
        size_t body_start  = header_end + 4;
        if (raw.size() >= body_start + content_len) break; // full body received
    }

    HttpRequest req = parse_request(raw);
    HttpResponse resp;

    if (!req.valid) {
        resp = HttpResponse::bad_request();
    } else if (req.method == "GET") {
        resp = handle_static_file(req, static_root);
    } else if (req.method == "POST") {
        // Echo the posted body back as JSON
        std::ostringstream json;
        json << "{\"method\":\"POST\",\"path\":\"" << req.path
             << "\",\"body\":\"" << req.body << "\"}";
        resp = HttpResponse::make(200, json.str(), "application/json");
    } else {
        resp = HttpResponse::method_not_allowed();
    }

    std::string response_str = resp.to_string();
    send(client_fd, response_str.c_str(), response_str.size(), 0);
    close(client_fd);
}
