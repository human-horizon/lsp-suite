#include <fstream>
#include "lsp.h"
#include "json-parser.h"

static JsonParser json_parser;

std::string LspServer::handle(const std::string& request) {
    simdjson::dom::parser parser;
    auto doc = parser.parse(request);

    std::string method;
    std::string id;

    for (auto [key, value] : doc.get_object()) {
        if (key == "method") {
            method = std::string(value.get_string().value());
        } else if (key == "id") {
            if (value.is_number()) {
                double num = value.get_double();
                id = std::to_string(static_cast<long long>(num));
            } else {
                id = std::string(value.get_string().value());
            }
        }
    }

    std::string result;
    if (method == "initialize") {
        result = handle_initialize();
    } else if (method == "textDocument/documentSymbol") {
        result = handle_document_symbol(request);
    } else if (method == "textDocument/hover") {
        result = handle_hover();
    } else if (method == "shutdown") {
        return build_response(id, "null");
    } else {
        return "";
    }

    return build_response(id, result);
}

std::string LspServer::handle_initialize() {
    return R"({"capabilities":{"textDocument":{"documentSymbol":{"hierarchicalDocumentSymbolSupport":true},"hoverProvider":true}},"serverInfo":{"name":"json-lsp","version":"0.1.0"}})";
}

std::string symbols_to_json(const std::vector<JsonSymbol>& symbols, int depth = 0) {
    std::string result = "[";
    bool first = true;
    for (const auto& sym : symbols) {
        if (!first) result += ",";
        first = false;

        result += "{";
        result += "\"name\":\"" + sym.name + "\",";
        result += "\"kind\":" + std::to_string(sym.kind) + ",";
        result += "\"detail\":\"" + sym.detail + "\",";
        result += "\"range\":{\"start\":{\"line\":0,\"character\":0},\"end\":{\"line\":" + std::to_string(depth) + ",\"character\":10}},";
        result += "\"selectionRange\":{\"start\":{\"line\":0,\"character\":0},\"end\":{\"line\":0,\"character\":10}}";

        if (!sym.children.empty()) {
            result += ",\"children\":" + symbols_to_json(sym.children, depth + 1);
        }

        result += "}";
    }
    result += "]";
    return result;
}

std::string LspServer::handle_document_symbol(const std::string& params) {
    std::string uri = extract_uri(params);
    std::string path = extract_path(uri);

    if (path.empty()) {
        return build_error("1", -32600, "Invalid params");
    }

    auto symbols = json_parser.parse_file(path);
    std::string symbols_json = symbols_to_json(symbols);

    return symbols_json;
}

std::string LspServer::handle_hover() {
    return R"({"contents":{"kind":"markdown","value":"JSON LSP Hover"}})";
}

std::string LspServer::build_response(const std::string& id, const std::string& result) {
    return R"({"jsonrpc":"2.0","id":)" + id + R"(,"result":)" + result + R"(})";
}

std::string LspServer::build_error(const std::string& id, int code, const std::string& message) {
    return R"({"jsonrpc":"2.0","id":)" + id + R"(,"error":{"code":)" + std::to_string(code) + R"(,"message":")" + message + R"("}})";
}

std::string LspServer::extract_uri(const std::string& params) {
    simdjson::dom::parser parser;
    auto doc = parser.parse(params);

    if (!doc.is_object()) {
        return "";
    }

    for (auto [key, value] : doc.get_object()) {
        if (key == "textDocument") {
            if (value.is_object()) {
                for (auto [td_key, td_value] : value.get_object()) {
                    if (td_key == "uri") {
                        return std::string(td_value.get_string().value());
                    }
                }
            }
        }
    }
    return "";
}

std::string LspServer::extract_path(const std::string& uri) {
    if (uri.find("file://") == 0) {
        return uri.substr(7);
    }
    return uri;
}
