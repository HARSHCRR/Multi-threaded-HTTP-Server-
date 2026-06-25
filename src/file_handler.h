#pragma once
#include "http_response.h"
#include "http_parser.h"
#include <string>

HttpResponse handle_static_file(const HttpRequest& req, const std::string& root_dir);
std::string mime_type(const std::string& path);
