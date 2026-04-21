/**
 * c-lsp - C Language Server using TCC parser
 * Uses TCC for parsing and semantic symbol extraction
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libtcc.h"

#define MAX_SYMBOLS 256
#define MAX_RESULT_SIZE 65536

typedef struct {
    char name[128];
    char type[64];
    int kind;
    unsigned int line;
    unsigned int column;
    unsigned int length;
} CSymbol;

static CSymbol symbols[MAX_SYMBOLS];
static int symbol_count = 0;

static void symbol_callback(void *ctx, const char *name, const void *val) {
    (void)ctx;
    (void)val;

    if (symbol_count >= MAX_SYMBOLS) return;
    if (!name || !name[0]) return;

    strncpy(symbols[symbol_count].name, name, sizeof(symbols[symbol_count].name) - 1);
    symbols[symbol_count].name[sizeof(symbols[symbol_count].name) - 1] = '\0';

    if (name[0] >= 'A' && name[0] <= 'Z') {
        strcpy(symbols[symbol_count].type, "macro");
        symbols[symbol_count].kind = 14;
    } else {
        strcpy(symbols[symbol_count].type, "function");
        symbols[symbol_count].kind = 12;
    }

    symbols[symbol_count].line = 1;
    symbols[symbol_count].column = 1;
    symbols[symbol_count].length = strlen(name);

    symbol_count++;
}

static void extract_symbols(TCCState *s) {
    symbol_count = 0;
    tcc_list_symbols(s, NULL, symbol_callback);
}

static void send_response(const char *id, const char *result) {
    printf("Content-Length: %zu\r\n\r\n", strlen(result));
    printf("{\"jsonrpc\":\"2.0\",\"id\":%s,\"result\":%s}", id, result);
    fflush(stdout);
}

static void send_error(const char *id, int code, const char *message) {
    char resp[512];
    snprintf(resp, sizeof(resp),
        "{\"jsonrpc\":\"2.0\",\"id\":%s,\"error\":{\"code\":%d,\"message\":\"%s\"}}",
        id, code, message);
    printf("Content-Length: %zu\r\n\r\n", strlen(resp));
    printf("%s", resp);
    fflush(stdout);
}

static void build_symbols_json(char *out, size_t max_len) {
    strcpy(out, "[");
    for (int i = 0; i < symbol_count; i++) {
        char entry[512];
        snprintf(entry, sizeof(entry),
            "%s{\"name\":\"%s\",\"kind\":%d,\"detail\":\"%s\",\"location\":{\"uri\":\"\",\"range\":{\"start\":{\"line\":%u,\"character\":%u},\"end\":{\"line\":%u,\"character\":%u}}}}",
            i > 0 ? "," : "",
            symbols[i].name,
            symbols[i].kind,
            symbols[i].type,
            symbols[i].line,
            symbols[i].column,
            symbols[i].line,
            symbols[i].column + symbols[i].length);
        strcat(out, entry);
    }
    strcat(out, "]");
}

static const char* get_file_path(const char *body, size_t body_len) {
    (void)body_len;
    const char *uri = strstr(body, "\"uri\"");
    if (!uri) return NULL;
    uri = strchr(uri + 5, '"');
    if (!uri) return NULL;
    uri++;
    const char *end = strchr(uri, '"');
    if (!end) return NULL;

    static char path[256];
    int path_len = (int)(end - uri);
    if (path_len >= (int)sizeof(path)) path_len = sizeof(path) - 1;
    memcpy(path, uri, path_len);
    path[path_len] = '\0';

    if (strncmp(path, "file://", 7) == 0) {
        return path + 7;
    }
    return path;
}

static void parse_c_file(const char *filepath) {
    FILE *f = fopen(filepath, "rb");
    if (!f) return;

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *content = malloc(len + 1);
    fread(content, 1, len, f);
    content[len] = '\0';
    fclose(f);

    TCCState *s = tcc_new();
    if (!s) {
        free(content);
        return;
    }

    tcc_set_options(s, "-std=c99");
    tcc_compile_string(s, content);

    extract_symbols(s);

    tcc_delete(s);
    free(content);
}

int main() {
    char header[256] = {0};
    char *body = NULL;
    size_t body_capacity = 0;

    while (1) {
        int header_len = 0;
        int content_length = 0;

        while (1) {
            int c = fgetc(stdin);
            if (c == EOF) {
                free(body);
                return 0;
            }
            if (c == '\n') {
                if (header_len == 1 && header[0] == '\r') {
                    header_len = 0;
                    break;
                }
                header[header_len] = '\0';
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
            body = (char*)realloc(body, content_length + 1);
            body_capacity = content_length + 1;
        }

        size_t total = 0;
        while (total < (size_t)content_length) {
            size_t n = fread(body + total, 1, content_length - total, stdin);
            if (n == 0) {
                free(body);
                return 0;
            }
            total += n;
        }
        body[content_length] = '\0';

        char method[64] = {0};
        char id[32] = {0};

        const char *s = body;
        const char *end = body + content_length;

        if (*s == '{') s++;

        while (s < end) {
            while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') s++;
            if (*s == '}') break;
            if (*s != '"') break;
            s++;
            const char *key_start = s;
            const char *key_end = strchr(s, '"');
            if (!key_end || key_end >= end) break;
            s = key_end + 1;

            while (*s == ' ' || *s == '\t') s++;
            if (*s != ':') break;
            s++;

            while (*s == ' ' || *s == '\t') s++;
            if (*s == '"') {
                s++;
                const char *val_start = s;
                const char *val_end = strchr(s, '"');
                if (!val_end || val_end >= end) break;
                s = val_end + 1;

                int key_len = (int)(key_end - key_start);
                int val_len = (int)(val_end - val_start);
                if (key_len > 0 && val_len > 0 && key_len < 64 && val_len < 64) {
                    char key[64];
                    memcpy(key, key_start, key_len);
                    key[key_len] = '\0';

                    char val[64];
                    memcpy(val, val_start, val_len);
                    val[val_len] = '\0';

                    if (strcmp(key, "method") == 0) {
                        strcpy(method, val);
                    } else if (strcmp(key, "id") == 0) {
                        strcpy(id, val);
                    }
                }
            } else if (*s >= '0' && *s <= '9') {
                int val_len = 0;
                while (s + val_len < end && *s >= '0' && *s <= '9') {
                    val_len++;
                    s++;
                }
                if (val_len > 0 && val_len < 32) {
                    int key_len = (int)(key_end - key_start);
                    if (key_len > 0 && key_len < 64) {
                        char key[64];
                        memcpy(key, key_start, key_len);
                        key[key_len] = '\0';
                        if (strcmp(key, "id") == 0) {
                            memcpy(id, s - val_len, val_len);
                            id[val_len] = '\0';
                        }
                    }
                }
            }

            while (*s == ' ' || *s == '\t') s++;
            if (*s == ',') s++;
        }

        if (strcmp(method, "initialize") == 0) {
            const char *result = "{\"capabilities\":{\"textDocument\":{\"documentSymbol\":{\"hierarchicalDocumentSymbolSupport\":false},\"hoverProvider\":true,\"definitionProvider\":true}},\"serverInfo\":{\"name\":\"c-lsp\",\"version\":\"0.1.0\"}}";
            send_response(id, result);
        } else if (strcmp(method, "textDocument/documentSymbol") == 0) {
            const char *filepath = get_file_path(body, content_length);
            if (filepath) {
                symbol_count = 0;
                parse_c_file(filepath);

                char result[MAX_RESULT_SIZE];
                build_symbols_json(result, sizeof(result));
                send_response(id, result);
            } else {
                send_error(id, -32602, "File not found");
            }
        } else if (strcmp(method, "textDocument/hover") == 0) {
            if (symbol_count > 0) {
                char result[MAX_RESULT_SIZE];
                snprintf(result, sizeof(result),
                    "{\"contents\":{\"kind\":\"markdown\",\"value\":\"C symbol (%d symbols found)\\n\\n- %s\"}}",
                    symbol_count, symbols[0].name);
                send_response(id, result);
            } else {
                send_response(id, "{\"contents\":{\"kind\":\"markdown\",\"value\":\"C symbol\"}}");
            }
        } else if (strcmp(method, "textDocument/definition") == 0) {
            if (symbol_count > 0) {
                char result[256];
                snprintf(result, sizeof(result),
                    "[{\"uri\":\"\",\"range\":{\"start\":{\"line\":%u,\"character\":%u},\"end\":{\"line\":%u,\"character\":%u}}}]",
                    symbols[0].line,
                    symbols[0].column,
                    symbols[0].line,
                    symbols[0].column + symbols[0].length);
                send_response(id, result);
            } else {
                send_response(id, "[]");
            }
        } else if (strcmp(method, "shutdown") == 0) {
            send_response(id, "null");
            break;
        }
    }

    free(body);
    return 0;
}