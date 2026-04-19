/**
 * c-lsp - C Language Server using TCC parser
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void* use_tcc_realloc(void *ptr, unsigned long size) {
    return realloc(ptr, size);
}

void use_tcc_free(void *ptr) {
    free(ptr);
}

#include "tcc.h"

void send_response(const char *id, const char *result) {
    printf("Content-Length: %zu\r\n\r\n", strlen(result));
    printf("{\"jsonrpc\":\"2.0\",\"id\":%s,\"result\":%s}", id, result);
    fflush(stdout);
}

const char* skip_ws(const char *s) {
    while (*s == ' ' || *s == '\n' || *s == '\r' || *s == '\t') s++;
    return s;
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
                if (header_len == 0) {
                    break;
                }
                if (strncmp(header, "Content-Length: ", 15) == 0) {
                    content_length = atoi(header + 15);
                }
                header_len = 0;
            } else if (c != '\r' && header_len < (int)(sizeof(header) - 1)) {
                header[header_len++] = c;
            }
        }

        if (content_length == 0) {
            continue;
        }

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
            s = skip_ws(s);
            if (*s == '}') break;
            if (*s != '"') break;
            s++;
            const char *key_start = s;
            const char *key_end = strchr(s, '"');
            if (!key_end || key_end >= end) break;
            s = key_end + 1;

            s = skip_ws(s);
            if (*s != ':') break;
            s++;

            s = skip_ws(s);
            if (*s == '"') {
                s++;
                const char *val_start = s;
                const char *val_end = strchr(s, '"');
                if (!val_end || val_end >= end) break;
                s = val_end + 1;

                int key_len = key_end - key_start;
                int val_len = val_end - val_start;
                if (key_len > 0 && val_len > 0) {
                    char key[64];
                    if (key_len >= (int)sizeof(key)) key_len = sizeof(key) - 1;
                    memcpy(key, key_start, key_len);
                    key[key_len] = '\0';

                    if (strcmp(key, "method") == 0) {
                        if (val_len >= (int)sizeof(method)) val_len = sizeof(method) - 1;
                        memcpy(method, val_start, val_len);
                        method[val_len] = '\0';
                    } else if (strcmp(key, "id") == 0) {
                        if (val_len >= (int)sizeof(id)) val_len = sizeof(id) - 1;
                        memcpy(id, val_start, val_len);
                        id[val_len] = '\0';
                    }
                }
            } else if (*s >= '0' && *s <= '9') {
                int val_len = 0;
                while (s + val_len < end && *s >= '0' && *s <= '9') {
                    val_len++;
                    s++;
                }
                if (val_len > 0) {
                    int key_len = key_end - key_start;
                    if (key_len > 0) {
                        char key[64];
                        if (key_len >= (int)sizeof(key)) key_len = sizeof(key) - 1;
                        memcpy(key, key_start, key_len);
                        key[key_len] = '\0';
                        if (strcmp(key, "id") == 0) {
                            if (val_len >= (int)sizeof(id)) val_len = sizeof(id) - 1;
                            memcpy(id, s - val_len, val_len);
                            id[val_len] = '\0';
                        }
                    }
                }
            }

            s = skip_ws(s);
            if (*s == ',') s++;
        }

        if (strcmp(method, "initialize") == 0) {
            const char *result = "{\"capabilities\":{\"textDocument\":{\"documentSymbol\":{\"hierarchicalDocumentSymbolSupport\":true},\"hoverProvider\":true}},\"serverInfo\":{\"name\":\"c-lsp\",\"version\":\"0.1.0\"}}";
            send_response(id, result);
        } else if (strcmp(method, "textDocument/documentSymbol") == 0) {
            send_response(id, "[]");
        } else if (strcmp(method, "textDocument/hover") == 0) {
            send_response(id, "{\"contents\":{\"kind\":\"markdown\",\"value\":\"C symbol\"}}");
        } else if (strcmp(method, "shutdown") == 0) {
            send_response(id, "null");
            break;
        }
    }

    free(body);
    return 0;
}