#include "http_parser.h"
#include <sstream>
#include <algorithm>

HttpRequest parse_request(const std::string& raw) {
    HttpRequest req;
    std::istringstream stream(raw);
    std::string line;

    // Parse request line: METHOD /path HTTP/1.1
    if (!std::getline(stream, line)) return req;
    if (!line.empty() && line.back() == '\r') line.pop_back();

    std::istringstream request_line(line);
    if (!(request_line >> req.method >> req.path >> req.version)) return req;

    // Parse headers
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) break; // blank line = end of headers

        auto colon = line.find(':');
        if (colon == std::string::npos) continue;

        std::string key = line.substr(0, colon);
        std::string val = line.substr(colon + 1);

        // Trim leading whitespace from value
        val.erase(0, val.find_first_not_of(" \t"));

        // Normalize header key to lowercase
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        req.headers[key] = val;
    }

    // Parse body (for POST)
    auto it = req.headers.find("content-length");
    if (it != req.headers.end()) {
        size_t len = std::stoul(it->second);
        req.body.resize(len);
        stream.read(req.body.data(), len);
    }

    req.valid = true;
    return req;
}
