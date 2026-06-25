#include "file_handler.h"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

std::string mime_type(const std::string& path) {
    auto ext_pos = path.rfind('.');
    if (ext_pos == std::string::npos) return "application/octet-stream";

    std::string ext = path.substr(ext_pos);
    if (ext == ".html" || ext == ".htm") return "text/html";
    if (ext == ".css")                   return "text/css";
    if (ext == ".js")                    return "application/javascript";
    if (ext == ".json")                  return "application/json";
    if (ext == ".png")                   return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".svg")                   return "image/svg+xml";
    if (ext == ".ico")                   return "image/x-icon";
    if (ext == ".txt")                   return "text/plain";
    return "application/octet-stream";
}

HttpResponse handle_static_file(const HttpRequest& req, const std::string& root_dir) {
    std::string url_path = req.path;

    // Strip query string
    auto q = url_path.find('?');
    if (q != std::string::npos) url_path = url_path.substr(0, q);

    // Default to index.html
    if (url_path == "/" || url_path.empty()) url_path = "/index.html";

    // Prevent path traversal: reject any ".." components
    if (url_path.find("..") != std::string::npos) {
        return HttpResponse::bad_request();
    }

    fs::path file_path = fs::path(root_dir) / url_path.substr(1);
    file_path = file_path.lexically_normal();

    // Ensure the resolved path is still inside root_dir
    fs::path root = fs::path(root_dir).lexically_normal();
    auto [root_end, _] = std::mismatch(root.begin(), root.end(), file_path.begin());
    if (root_end != root.end()) {
        return HttpResponse::bad_request();
    }

    if (!fs::exists(file_path) || !fs::is_regular_file(file_path)) {
        return HttpResponse::not_found();
    }

    std::ifstream file(file_path, std::ios::binary);
    if (!file) return HttpResponse::internal_error();

    std::ostringstream ss;
    ss << file.rdbuf();

    return HttpResponse::make(200, ss.str(), mime_type(file_path.string()));
}
