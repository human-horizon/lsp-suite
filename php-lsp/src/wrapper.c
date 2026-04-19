#include "tree-sitter-php.h"

extern const TSLanguage *tree_sitter_php_only(void);

const TSLanguage *tree_sitter_php(void) {
    return tree_sitter_php_only();
}
