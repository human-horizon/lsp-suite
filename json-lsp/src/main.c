/**
 * json-lsp - JSON Language Server (Pure C)
 * Uses manual JSON parsing for LSP protocol
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SYMBOLS 256
#define MAX_RESULT_SIZE 65536

typedef struct {
    char name[128];
    char type[32];
    char detail[128];
    int kind;
    size_t offset;
    size_t length;
} JsonSymbol;

static JsonSymbol symbols[MAX_SYMBOLS];
static int symbol_count = 0;

static const char* skip_ws(const char *s) {
    while (*s == ' ' || *s == '\n' || *s == '\r' || *s == '\t') s++;
    return s;
}

static const char* skip_string(const char *s) {
    if (*s != '"') return s;
    s++;
    while (*s && *s != '"') {
        if (*s == '\\') s++;
        s++;
    }
    return *s ? s + 1 : s;
}

static const char* skip_value(const char *s);

static const char* skip_object(const char *s) {
    if (*s != '{') return s;
    s++;
    int depth = 1;
    while (*s && depth > 0) {
        if (*s == '{' || *s == '[') {
            depth++;
            s++;
        } else if (*s == '}' || *s == ']') {
            depth--;
            s++;
        } else if (*s == '"') {
            s = skip_string(s);
        } else {
            s++;
        }
    }
    return s;
}

static const char* skip_array(const char *s) {
    return skip_object(s);
}

static const char* skip_value(const char *s) {
    s = skip_ws(s);
    if (*s == '"') {
        return skip_string(s);
    } else if (*s == '{') {
        return skip_object(s);
    } else if (*s == '[') {
        return skip_array(s);
    } else {
        while (*s && *s != ',' && *s != '}' && *s != ']') s++;
        return s;
    }
}

static int count_object_items(const char *s) {
    if (*s != '{') return 0;
    s++;
    int count = 0;
    int depth = 1;
    while (*s && depth > 0) {
        if (*s == '{' || *s == '[') {
            depth++;
            s++;
        } else if (*s == '}' || *s == ']') {
            depth--;
            s++;
        } else if (*s == '"') {
            s++;
            while (*s && *s != '"') {
                if (*s == '\\') s++;
                s++;
            }
            if (*s == '"') s++;
            s = skip_ws(s);
            if (*s == ':') {
                s++;
                s = skip_value(s);
                count++;
                s = skip_ws(s);
                if (*s == ',') s++;
            }
        } else {
            s++;
        }
    }
    return count;
}

static int count_array_items(const char *s) {
    if (*s != '[') return 0;
    s++;
    int count = 0;
    int depth = 1;
    while (*s && depth > 0) {
        if (*s == '{' || *s == '[') {
            depth++;
            s++;
        } else if (*s == '}' || *s == ']') {
            depth--;
            s++;
        } else if (*s == '"') {
            s = skip_string(s);
            count++;
            s = skip_ws(s);
            if (*s == ',') s++;
        } else {
            s = skip_value(s);
            count++;
            s = skip_ws(s);
            if (*s == ',') s++;
        }
    }
    return count;
}

static const char* get_object_key(const char *s, int index, char *out, int max_len) {
    if (*s != '{') return NULL;
    s++;
    int count = 0;
    while (*s) {
        s = skip_ws(s);
        if (*s == '}') return NULL;
        if (*s != '"') break;
        s++;
        const char *start = s;
        while (*s && *s != '"') {
            if (*s == '\\') s++;
            s++;
        }
        int len = (int)(s - start);
        if (len > max_len - 1) len = max_len - 1;
        memcpy(out, start, len);
        out[len] = '\0';
        if (*s == '"') s++;
        s = skip_ws(s);
        if (*s == ':') {
            s++;
            if (count == index) {
                return s;
            }
            s = skip_value(s);
            count++;
            s = skip_ws(s);
            if (*s == ',') s++;
        }
    }
    return NULL;
}

static const char* get_array_element(const char *s, int index) {
    if (*s != '[') return NULL;
    s++;
    int count = 0;
    while (*s) {
        s = skip_ws(s);
        if (*s == ']') return NULL;
        const char *start = s;
        if (*s == '{' || *s == '[') {
            s = skip_value(s);
        } else if (*s == '"') {
            s = skip_string(s);
        } else {
            while (*s && *s != ',' && *s != ']') s++;
        }
        if (count == index) return start;
        count++;
        s = skip_ws(s);
        if (*s == ',') s++;
    }
    return NULL;
}

static int get_type_kind(const char *s) {
    s = skip_ws(s);
    if (*s == '{') return 3;
    if (*s == '[') return 4;
    return 1;
}

static void parse_json_symbols(const char *content) {
    symbol_count = 0;

    const char *s = skip_ws(content);

    if (*s == '[') {
        int count = count_array_items(s);
        for (int i = 0; i < count && symbol_count < MAX_SYMBOLS; i++) {
            const char *elem = get_array_element(s, i);
            if (elem) {
                snprintf(symbols[symbol_count].name, sizeof(symbols[symbol_count].name), "[%d]", i);
                symbols[symbol_count].kind = get_type_kind(elem);
                snprintf(symbols[symbol_count].type, sizeof(symbols[symbol_count].type), "element");
                snprintf(symbols[symbol_count].detail, sizeof(symbols[symbol_count].detail), "element");
                symbol_count++;
            }
        }
    } else if (*s == '{') {
        int count = count_object_items(s);
        for (int i = 0; i < count && symbol_count < MAX_SYMBOLS; i++) {
            char key[128];
            const char *val = get_object_key(s, i, key, sizeof(key));
            if (val) {
                strncpy(symbols[symbol_count].name, key, sizeof(symbols[symbol_count].name) - 1);
                symbols[symbol_count].name[sizeof(symbols[symbol_count].name) - 1] = '\0';
                symbols[symbol_count].kind = get_type_kind(val);

                if (*val == '{') {
                    snprintf(symbols[symbol_count].type, sizeof(symbols[symbol_count].type), "object");
                    int inner = count_object_items(val);
                    snprintf(symbols[symbol_count].detail, sizeof(symbols[symbol_count].detail), "object{%d}", inner);
                } else if (*val == '[') {
                    snprintf(symbols[symbol_count].type, sizeof(symbols[symbol_count].type), "array");
                    int inner = count_array_items(val);
                    snprintf(symbols[symbol_count].detail, sizeof(symbols[symbol_count].detail), "array[%d]", inner);
                } else if (*val == '"') {
                    snprintf(symbols[symbol_count].type, sizeof(symbols[symbol_count].type), "string");
                    snprintf(symbols[symbol_count].detail, sizeof(symbols[symbol_count].detail), "string");
                } else {
                    snprintf(symbols[symbol_count].type, sizeof(symbols[symbol_count].type), "value");
                    snprintf(symbols[symbol_count].detail, sizeof(symbols[symbol_count].detail), "value");
                }
                symbol_count++;
            }
        }
    }
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
    (void)max_len;
    strcpy(out, "[");
    for (int i = 0; i < symbol_count; i++) {
        char entry[512];
        snprintf(entry, sizeof(entry),
            "%s{\"name\":\"%s\",\"kind\":%d,\"detail\":\"%s\",\"location\":{\"uri\":\"\",\"range\":{\"start\":{\"line\":0,\"character\":0},\"end\":{\"line\":0,\"character\":0}}}}",
            i > 0 ? "," : "",
            symbols[i].name,
            symbols[i].kind,
            symbols[i].detail);
        strcat(out, entry);
    }
    strcat(out, "]");
}

int main(void) {
    char header[256];
    char *body = NULL;
    size_t body_capacity = 0;
    char *file_content = NULL;
    char result[MAX_RESULT_SIZE];

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
                if (*s == '"') {
                    s++;
                    const char *start = s;
                    while (*s && *s != '"') s++;
                    size_t len = s - start;
                    if (len > sizeof(method) - 1) len = sizeof(method) - 1;
                    memcpy(method, start, len);
                    method[len] = '\0';
                    if (*s == '"') s++;
                }
            } else if (strcmp(key, "id") == 0) {
                s = skip_ws(s);
                if (*s == '"') {
                    s++;
                    const char *start = s;
                    while (*s && *s != '"') s++;
                    size_t len = s - start;
                    if (len > sizeof(id) - 1) len = sizeof(id) - 1;
                    memcpy(id, start, len);
                    id[len] = '\0';
                    if (*s == '"') s++;
                } else if (*s >= '0' && *s <= '9') {
                    int i = 0;
                    while (*s >= '0' && *s <= '9' && i < (int)sizeof(id) - 1) {
                        id[i++] = *s++;
                    }
                    id[i] = '\0';
                }
            } else {
                s = skip_value(s);
            }

            s = skip_ws(s);
            if (*s == ',') s++;
        }

        if (strcmp(method, "initialize") == 0) {
            const char *result = "{\"capabilities\":{\"textDocument\":{\"documentSymbol\":{\"hierarchicalDocumentSymbolSupport\":true}},\"hoverProvider\":true},\"serverInfo\":{\"name\":\"json-lsp\",\"version\":\"0.1.0\"}}";
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
                        int path_len = (int)(end - uri);
                        if (path_len < (int)sizeof(path)) {
                            memcpy(path, uri, path_len);
                            path[path_len] = '\0';
                            if (strncmp(path, "file://", 7) == 0) {
                                FILE *f = fopen(path + 7, "rb");
                                if (f) {
                                    fseek(f, 0, SEEK_END);
                                    long len = ftell(f);
                                    fseek(f, 0, SEEK_SET);
                                    file_content = realloc(file_content, len + 1);
                                    fread(file_content, 1, len, f);
                                    file_content[len] = '\0';
                                    fclose(f);
                                }
                            }
                        }
                    }
                }
            }

            if (!file_content) {
                send_error(id, -32602, "File not found");
            } else {
                parse_json_symbols(file_content);
                build_symbols_json(result, sizeof(result));
                send_response(id, result);
            }
            free(file_content);
            file_content = NULL;
        } else if (strcmp(method, "textDocument/hover") == 0) {
            send_response(id, "{\"contents\":{\"kind\":\"markdown\",\"value\":\"JSON symbol\"}}");
        } else if (strcmp(method, "shutdown") == 0) {
            send_response(id, "null");
            break;
        }
    }

cleanup:
    free(body);
    free(file_content);
    return 0;
}