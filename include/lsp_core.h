#ifndef LSP_CORE_H
#define LSP_CORE_H

#include <tree_sitter/api.h>

/* JSON-RPC utilities */
const char* lsp_skip_ws(const char *s);
const char* lsp_parse_string(const char *s, char *out, int max_len);
void lsp_skip_value(const char **s);

/* Response helpers */
void lsp_send_response(const char *id, const char *result);
void lsp_send_error(const char *id, int code, const char *message);

/* File utilities */
char* lsp_read_file(const char *path);
char* lsp_extract_path_from_uri(const char *uri);

/* Symbol extraction */
typedef struct {
    unsigned int start_byte;
    unsigned int end_byte;
    int kind;
} LSPSymbol;

void lsp_extract_symbols(TSNode node, const TSLanguage *lang,
                        LSPSymbol *symbols, int *symbol_count,
                        const char **symbol_names);

const char* lsp_kind_to_name(int kind);

#endif
