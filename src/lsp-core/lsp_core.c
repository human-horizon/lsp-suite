#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tree_sitter/api.h>
#include "lsp_core.h"

/* JSON-RPC utilities */
const char* lsp_skip_ws(const char *s) {
    while (*s == ' ' || *s == '\n' || *s == '\r' || *s == '\t') s++;
    return s;
}

const char* lsp_parse_string(const char *s, char *out, int max_len) {
    if (*s != '"') return NULL;
    s++;
    int i = 0;
    while (*s && *s != '"' && i < max_len - 1) {
        if (*s == '\\') s++;
        out[i++] = *s++;
    }
    out[i] = '\0';
    if (*s == '"') s++;
    return s;
}

void lsp_skip_value(const char **s) {
    *s = lsp_skip_ws(*s);

    if (**s == '"') {
        (*s)++;
        while (**s && **s != '"') {
            if (**s == '\\') (*s)++;
            (*s)++;
        }
        if (**s == '"') (*s)++;
    } else if (**s == '{') {
        int depth = 1;
        (*s)++;
        while (**s && depth > 0) {
            if (**s == '{') { depth++; (*s)++; }
            else if (**s == '}') {
                depth--;
                if (depth == 0) { (*s)++; break; }
                (*s)++;
            }
            else if (**s == '"') {
                (*s)++;
                while (**s && **s != '"') {
                    if (**s == '\\') (*s)++;
                    (*s)++;
                }
                if (**s == '"') (*s)++;
            } else {
                (*s)++;
            }
        }
    } else if (**s == '[') {
        int depth = 1;
        (*s)++;
        while (**s && depth > 0) {
            if (**s == '[') { depth++; (*s)++; }
            else if (**s == ']') {
                depth--;
                if (depth == 0) { (*s)++; break; }
                (*s)++;
            }
            else if (**s == '"') {
                (*s)++;
                while (**s && **s != '"') {
                    if (**s == '\\') (*s)++;
                    (*s)++;
                }
                if (**s == '"') (*s)++;
            } else {
                (*s)++;
            }
        }
    } else if (**s >= '0' && **s <= '9') {
        while (**s >= '0' && **s <= '9') (*s)++;
    } else {
        (*s)++;
    }

    *s = lsp_skip_ws(*s);
    if (**s == ',') (*s)++;
}

/* Response helpers */
void lsp_send_response(const char *id, const char *result) {
    char resp[8192];
    int len = snprintf(resp, sizeof(resp),
        "{\"jsonrpc\":\"2.0\",\"id\":%s,\"result\":%s}", id ? id : "null", result);
    printf("Content-Length: %d\r\n\r\n", len);
    printf("%s", resp);
    fflush(stdout);
}

void lsp_send_error(const char *id, int code, const char *message) {
    char resp[512];
    snprintf(resp, sizeof(resp),
        "{\"jsonrpc\":\"2.0\",\"id\":%s,\"error\":{\"code\":%d,\"message\":\"%s\"}}",
        id ? id : "null", code, message);
    printf("Content-Length: %zu\r\n\r\n", strlen(resp));
    printf("%s", resp);
    fflush(stdout);
}

/* File utilities */
char* lsp_read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *content = malloc(len + 1);
    if (!content) { fclose(f); return NULL; }
    fread(content, 1, len, f);
    content[len] = '\0';
    fclose(f);
    return content;
}

char* lsp_extract_path_from_uri(const char *uri) {
    if (!uri) return NULL;
    if (strncmp(uri, "file://", 7) != 0) return NULL;
    return strdup(uri + 7);
}

/* Symbol extraction - base implementation, language-specific overrides possible */
void lsp_extract_symbols(TSNode node, const TSLanguage *lang,
                        LSPSymbol *symbols, int *symbol_count,
                        const char **symbol_names) {
    (void)lang;
    (void)symbols;
    (void)symbol_count;
    (void)symbol_names;
}

const char* lsp_kind_to_name(int kind) {
    switch (kind) {
        case 1: return "function";
        case 2: return "arrow";
        case 3: return "class";
        case 4: return "variable";
        case 5: return "method";
        case 6: return "module";
        case 7: return "enum";
        case 8: return "interface";
        case 9: return "type";
        default: return "unknown";
    }
}
