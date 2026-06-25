#pragma once
#include <string>
#include <unordered_map>

struct HttpResponse {
    int status_code = 200;
    std::string status_text = "OK";
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    std::string to_string() const;

    static HttpResponse make(int code, const std::string& body,
                             const std::string& content_type = "text/plain");
    static HttpResponse not_found();
    static HttpResponse bad_request();
    static HttpResponse method_not_allowed();
    static HttpResponse internal_error();
};
