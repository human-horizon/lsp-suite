#pragma once

#include <string>
#include <vector>
#include <simdjson.h>

struct JsonSymbol {
    std::string name;
    std::string type;        // "string", "number", "boolean", "object", "array", "null"
    std::string detail;     // "array[10]", "object{...}", etc
    int kind;               // LSP SymbolKind: 1=variable, 3=object, 4=array
    size_t offset;          // byte offset in file for this key
    size_t length;          // byte length of this subtree
    std::vector<JsonSymbol> children;
};

class JsonParser {
public:
    JsonParser();
    ~JsonParser();

    // Parse file and get top-level keys
    std::vector<JsonSymbol> parse_file(const std::string& path);

    // Get children of a specific symbol
    std::vector<JsonSymbol> get_children(const std::string& path, size_t offset, size_t length);

    // Get type info for visible range in editor
    struct TypeInfo {
        size_t line;
        size_t start_col;
        size_t end_col;
        std::string type;
        std::string value_preview;
    };
    std::vector<TypeInfo> get_types_in_range(const std::string& path, size_t start_line, size_t end_line);

private:
    simdjson::ondemand::parser* parser;
};
