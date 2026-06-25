#include "http_response.h"
#include <sstream>

std::string HttpResponse::to_string() const {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
    for (auto& [k, v] : headers) {
        oss << k << ": " << v << "\r\n";
    }
    oss << "Content-Length: " << body.size() << "\r\n";
    oss << "Connection: close\r\n";
    oss << "\r\n";
    oss << body;
    return oss.str();
}

HttpResponse HttpResponse::make(int code, const std::string& body,
                                const std::string& content_type) {
    HttpResponse r;
    r.status_code = code;
    r.body = body;
    r.headers["Content-Type"] = content_type;

    static const std::unordered_map<int, std::string> texts = {
        {200, "OK"}, {201, "Created"}, {204, "No Content"},
        {400, "Bad Request"}, {404, "Not Found"},
        {405, "Method Not Allowed"}, {500, "Internal Server Error"}
    };
    auto it = texts.find(code);
    r.status_text = (it != texts.end()) ? it->second : "Unknown";
    return r;
}

HttpResponse HttpResponse::not_found() {
    return make(404, "<h1>404 Not Found</h1>", "text/html");
}

HttpResponse HttpResponse::bad_request() {
    return make(400, "<h1>400 Bad Request</h1>", "text/html");
}

HttpResponse HttpResponse::method_not_allowed() {
    return make(405, "<h1>405 Method Not Allowed</h1>", "text/html");
}

HttpResponse HttpResponse::internal_error() {
    return make(500, "<h1>500 Internal Server Error</h1>", "text/html");
}
