#pragma once

#include <string>
#include <simdjson.h>

class LspServer {
public:
    std::string handle(const std::string& request);

private:
    std::string handle_initialize();
    std::string handle_document_symbol(const std::string& params);
    std::string handle_hover();

    std::string build_response(const std::string& id, const std::string& result);
    std::string build_error(const std::string& id, int code, const std::string& message);
    std::string extract_uri(const std::string& params);
    std::string extract_path(const std::string& uri);
};
