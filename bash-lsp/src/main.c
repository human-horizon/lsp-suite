/**
 * bash-lsp - Bash Language Server using tree-sitter
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tree_sitter/api.h>
#include "tree-sitter-bash.h"

static TSParser *parser = NULL;

typedef struct {
    const char *name;
    unsigned int start_byte;
    unsigned int end_byte;
    int kind;
} Symbol;

static Symbol symbols[256];
static int symbol_count = 0;

void send_response(const char *id, const char *result) {
    printf("Content-Length: %zu\r\n\r\n", strlen(result));
    printf("{\"jsonrpc\":\"2.0\",\"id\":%s,\"result\":%s}", id, result);
    fflush(stdout);
}

void send_error(const char *id, int code, const char *message) {
    char resp[512];
    snprintf(resp, sizeof(resp),
        "{\"jsonrpc\":\"2.0\",\"id\":%s,\"error\":{\"code\":%d,\"message\":\"%s\"}}",
        id, code, message);
    printf("Content-Length: %zu\r\n\r\n", strlen(resp));
    printf("%s", resp);
    fflush(stdout);
}

void extract_symbols(TSNode node) {
    if (symbol_count >= 256) return;

    TSSymbol sym = ts_node_symbol(node);
    const char *name = ts_language_symbol_name(ts_parser_language(parser), sym);

    if (strcmp(name, "function_definition") == 0 ||
        strcmp(name, "command") == 0 ||
        strcmp(name, "if_statement") == 0 ||
        strcmp(name, "case_statement") == 0) {

        unsigned int start = ts_node_start_byte(node);
        unsigned int end = ts_node_end_byte(node);

        symbols[symbol_count].name = name;
        symbols[symbol_count].start_byte = start;
        symbols[symbol_count].end_byte = end;
        symbols[symbol_count].kind = 1;
        symbol_count++;
    }

    unsigned int child_count = ts_node_child_count(node);
    for (unsigned int i = 0; i < child_count; i++) {
        extract_symbols(ts_node_child(node, i));
    }
}

const char* skip_ws(const char *s) {
    while (*s == ' ' || *s == '\n' || *s == '\r' || *s == '\t') s++;
    return s;
}

const char* parse_string(const char *s, char *out, int max_len) {
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

static void skip_value(const char **s) {
    *s = skip_ws(*s);
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
            if (**s == '{') depth++;
            else if (**s == '}') depth--;
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
            if (**s == '[') depth++;
            else if (**s == ']') depth--;
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
    }
}

int main() {
    parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_bash());

    char header[256];
    char *body = NULL;
    size_t body_capacity = 0;
    char *source = NULL;

    while (1) {
        int header_len = 0;
        int content_length = 0;
        memset(header, 0, sizeof(header));

        while (1) {
            char c;
            if (fread(&c, 1, 1, stdin) != 1) goto cleanup;
            if (c == '\n') {
                header[header_len] = '\0';
                if (header_len == 1 && header[0] == '\r') break;
                if (header_len == 0) break;
                if (strncmp(header, "Content-Length: ", 15) == 0) {
                    content_length = atoi(header + 15);
                }
                header_len = 0;
            } else if (c != '\r' && header_len < (int)(sizeof(header) - 1)) {
                header[header_len++] = c;
            }
        }

        if (content_length == 0) continue;

        if ((size_t)content_length >= body_capacity) {
            body = realloc(body, content_length + 1);
            body_capacity = content_length + 1;
        }

        if (fread(body, 1, content_length, stdin) != (size_t)content_length) goto cleanup;
        body[content_length] = '\0';

        const char *s = skip_ws(body);
        if (*s != '{') continue;

        char method[64] = {0};
        char id[32] = {0};

        s++;

        while (*s) {
            s = skip_ws(s);
            if (*s == '}') break;
            if (*s != '"') break;
            s++;

            char key[32];
            const char *end = strchr(s, '"');
            if (!end || (size_t)(end - s) >= sizeof(key)) break;
            memcpy(key, s, end - s);
            key[end - s] = '\0';
            s = end + 1;

            s = skip_ws(s);
            if (*s != ':') break;
            s++;
            s = skip_ws(s);

            if (strcmp(key, "method") == 0) {
                char val[64] = {0};
                s = parse_string(s, val, sizeof(val));
                if (s) strncpy(method, val, sizeof(method) - 1);
            } else if (strcmp(key, "id") == 0) {
                s = skip_ws(s);
                if (*s == '"') {
                    s = parse_string(s, id, sizeof(id));
                } else if (*s >= '0' && *s <= '9') {
                    int i = 0;
                    while (*s >= '0' && *s <= '9' && i < (int)sizeof(id) - 1) {
                        id[i++] = *s++;
                    }
                    id[i] = '\0';
                }
            } else {
                skip_value(&s);
            }

            s = skip_ws(s);
            if (*s == ',') s++;
        }

        if (strcmp(method, "initialize") == 0) {
            const char *result = "{\"capabilities\":{\"textDocument\":{\"documentSymbol\":{\"hierarchicalDocumentSymbolSupport\":true},\"hoverProvider\":true}},\"serverInfo\":{\"name\":\"bash-lsp\",\"version\":\"0.1.0\"}}";
            send_response(id, result);
        } else if (strcmp(method, "textDocument/documentSymbol") == 0) {
            const char *uri = strstr(body, "\"uri\"");
            if (uri) {
                uri = strchr(uri + 5, '"');
                if (uri) {
                    uri++;
                    const char *end = strchr(uri, '"');
                    if (end) {
                        char path[256];
                        int path_len = end - uri;
                        if (path_len < (int)sizeof(path)) {
                            memcpy(path, uri, path_len);
                            path[path_len] = '\0';
                            if (strncmp(path, "file://", 7) == 0) {
                                FILE *f = fopen(path + 7, "rb");
                                if (f) {
                                    fseek(f, 0, SEEK_END);
                                    long len = ftell(f);
                                    fseek(f, 0, SEEK_SET);
                                    source = realloc(source, len + 1);
                                    fread(source, 1, len, f);
                                    source[len] = '\0';
                                    fclose(f);
                                }
                            }
                        }
                    }
                }
            }

            if (!source) {
                send_error(id, -32602, "File not found");
            } else {
                symbol_count = 0;
                TSTree *tree = ts_parser_parse_string(parser, NULL, source, strlen(source));
                TSNode root = ts_tree_root_node(tree);
                extract_symbols(root);

                char result[8192] = "[";
                for (int i = 0; i < symbol_count; i++) {
                    char entry[256];
                    snprintf(entry, sizeof(entry),
                        "%s{\"name\":\"%s\",\"kind\":%d,\"location\":{\"uri\":\"\",\"range\":{\"start\":{\"line\":0,\"character\":%u},\"end\":{\"line\":0,\"character\":%u}}}}",
                        i > 0 ? "," : "",
                        symbols[i].name,
                        symbols[i].kind,
                        symbols[i].start_byte,
                        symbols[i].end_byte);
                    strcat(result, entry);
                }
                strcat(result, "]");

                send_response(id, result);
                ts_tree_delete(tree);
            }
            free(source);
            source = NULL;
        } else if (strcmp(method, "textDocument/hover") == 0) {
            send_response(id, "{\"contents\":{\"kind\":\"markdown\",\"value\":\"Bash symbol\"}}");
        } else if (strcmp(method, "shutdown") == 0) {
            send_response(id, "null");
            break;
        }
    }

cleanup:
    free(body);
    free(source);
    ts_parser_delete(parser);
    return 0;
}