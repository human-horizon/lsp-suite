/**
 * python-lsp - Python Language Server using tree-sitter
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tree_sitter/api.h>
#include "tree-sitter-python.h"

static TSParser *parser = NULL;

void send_response(const char *id, const char *result) {
    printf("Content-Length: %zu\r\n\r\n", strlen(result));
    printf("{\"jsonrpc\":\"2.0\",\"id\":%s,\"result\":%s}", id, result);
    fflush(stdout);
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

int main() {
    parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_python());

    char header[256];
    char *body = NULL;
    size_t body_capacity = 0;

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
                while (*s && *s != ',' && *s != '}') s++;
            }

            s = skip_ws(s);
            if (*s == ',') s++;
        }

        if (strcmp(method, "initialize") == 0) {
            const char *result = "{\"capabilities\":{\"textDocument\":{\"documentSymbol\":{\"hierarchicalDocumentSymbolSupport\":true},\"hoverProvider\":true}},\"serverInfo\":{\"name\":\"python-lsp\",\"version\":\"0.1.0\"}}";
            send_response(id, result);
        } else if (strcmp(method, "shutdown") == 0) {
            send_response(id, "null");
            break;
        }
    }

cleanup:
    free(body);
    ts_parser_delete(parser);
    return 0;
}