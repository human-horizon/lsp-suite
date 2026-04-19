#include "json-parser.h"
#include <fstream>

JsonParser::JsonParser() {
}

JsonParser::~JsonParser() {
}

std::vector<JsonSymbol> JsonParser::parse_file(const std::string& path) {
    std::vector<JsonSymbol> result;

    std::ifstream file(path);
    if (!file.is_open()) {
        return result;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    simdjson::dom::parser parser;
    auto doc = parser.parse(content);

    if (doc.is_array()) {
        size_t idx = 0;
        for (auto element : doc.get_array()) {
            JsonSymbol sym;
            sym.name = "[" + std::to_string(idx) + "]";
            if (element.is_object()) {
                sym.type = "object";
                sym.detail = "object";
                sym.kind = 3;
            } else if (element.is_array()) {
                size_t count = 0;
                for (auto _ : element.get_array()) count++;
                sym.type = "array";
                sym.detail = "array[" + std::to_string(count) + "]";
                sym.kind = 4;
            } else if (element.is_string()) {
                sym.type = "string";
                sym.detail = "string";
                sym.kind = 1;
            } else if (element.is_number()) {
                sym.type = "number";
                sym.detail = "number";
                sym.kind = 1;
            } else if (element.is_bool()) {
                sym.type = "boolean";
                sym.detail = "boolean";
                sym.kind = 1;
            } else {
                sym.type = "null";
                sym.detail = "null";
                sym.kind = 1;
            }
            result.push_back(sym);
            idx++;
        }
    } else if (doc.is_object()) {
        for (auto [name, value] : doc.get_object()) {
            JsonSymbol sym;
            sym.name = std::string(name);
            if (value.is_object()) {
                size_t count = 0;
                for (auto _ : value.get_object()) count++;
                sym.type = "object";
                sym.detail = "object{" + std::to_string(count) + "}";
                sym.kind = 3;
            } else if (value.is_array()) {
                size_t count = 0;
                for (auto _ : value.get_array()) count++;
                sym.type = "array";
                sym.detail = "array[" + std::to_string(count) + "]";
                sym.kind = 4;
            } else if (value.is_string()) {
                sym.type = "string";
                sym.detail = "string";
                sym.kind = 1;
            } else if (value.is_number()) {
                sym.type = "number";
                sym.detail = "number";
                sym.kind = 1;
            } else if (value.is_bool()) {
                sym.type = "boolean";
                sym.detail = "boolean";
                sym.kind = 1;
            } else {
                sym.type = "null";
                sym.detail = "null";
                sym.kind = 1;
            }
            result.push_back(sym);
        }
    }

    return result;
}

std::vector<JsonSymbol> JsonParser::get_children(const std::string& path, size_t offset, size_t length) {
    std::vector<JsonSymbol> result;
    // Placeholder for hierarchical expansion
    return result;
}

std::vector<JsonParser::TypeInfo> JsonParser::get_types_in_range(const std::string& path, size_t start_line, size_t end_line) {
    std::vector<TypeInfo> result;
    // Placeholder for editor types
    return result;
}
