#include "../dynamic_array/dynamic_array.h"
#include <stdio.h>
#define STRING_IMPLEMENTATION
#include "../dynamic_array/string.h"
#define LEXER_IMPLEMENTATION
#include "lexer.h"

const char *sl_comments[] = {
    "//",
};

const multi_line_comments ml_comments[] = {
    {"/*", "*/"},
};

const char *keywords[] = {
    "class",
    "let",
    "function",
    "return",
};

const char *puncts[] = {
    "+",
    "*",
    "(",
    ")",
    "{",
    "}",
    "==",
    "=",
    ";",
};

int main()
{
    const char* file_path = "test.jack";

    string_builder* sb = sb_init(NULL);
    read_file(file_path, sb);
    string_view content = sb_to_sv(sb);

    lexer l = lexer_create(file_path, content);
    l.keywords = keywords;
    l.keywords_count = arr_count(keywords);
    l.puncts = puncts;
    l.puncts_count = arr_count(puncts);
    l.sl_comments = sl_comments;
    l.sl_comments_count = arr_count(sl_comments);
    l.ml_comments = ml_comments;
    l.ml_comments_count = arr_count(ml_comments);

    token t = {0};
    while(get_token(&l, &t)) {
        printf("%s:%zu:%zu: [%s] %.*s\n", t.loc.file_path, t.loc.row, t.loc.col, lexer_kind_name(t.id), (int)t.src.count, t.src.data);
    }

    if (t.id != LEXER_END) {
        printf("ERROR: %s:%zu:%zu: Expected: %s, but got %s: %.*s\n", t.loc.file_path, t.loc.row, t.loc.col,  lexer_kind_name(LEXER_END), lexer_kind_name(t.id), (int)t.src.count, t.src.data);
    }

    sb_free(sb);

    return 0;
}