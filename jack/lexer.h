#pragma once
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifndef STRING_IMPLEMENTATION
    #define STRING_IMPLEMENTATION
#endif // STRING_IMPLEMENTATION
#include "../dynamic_array/string.h"

#include "error.h"

typedef struct {
    const char *opening;
    const char *closing;
} multi_line_comments;

typedef struct {
    const char* file_path;
    size_t row;
    size_t col;
} location;

typedef enum {
    LEXER_INVALID,
    LEXER_END,
    LEXER_INT,
    LEXER_SYMBOL,
    LEXER_KEYWORD,
    LEXER_PUNCT,
    LEXER_COUNT_KINDS,
} lexer_kind;

typedef struct {
    lexer_kind id;
    string_view src;

    union {
        int int_value;
    };

    location loc;
} lexer_token;    

typedef struct {
    string_view content;

    size_t cur;
    size_t bol;
    size_t row;

    const char **puncts;
    size_t puncts_count;
    const char **keywords;
    size_t keywords_count;
    const char **sl_comments;
    size_t sl_comments_count;
    const multi_line_comments *ml_comments;
    size_t ml_comments_count;

    const char *file_path;
} lexer;

lexer lexer_create(const char *file_path, string_view content);
const char* lexer_kind_name(lexer_kind k);
Error* lexer_expect_id(lexer *l, lexer_token t, lexer_kind id);
Error* lexer_expect_cstr(lexer_token t, const char* cstr);
bool lexer_get_token(lexer* l, lexer_token* t);

#ifdef LEXER_IMPLEMENTATION
const char *lexer_kind_names[LEXER_COUNT_KINDS] = {
    [LEXER_INVALID] = "INVALID",
    [LEXER_INT]     = "INT",
    [LEXER_END]     = "END",
    [LEXER_SYMBOL]  = "SYMBOL",
    [LEXER_KEYWORD] = "KEYWORD",
    [LEXER_PUNCT]   = "PUNCT",
};

lexer lexer_create(const char *file_path, string_view content)
{
    return (lexer) {
        .file_path = file_path,
        .content = content,
    };
}

Error* lexer_expect_id(lexer *l, lexer_token t, lexer_kind id)
{
    if (t.id != id) {
        return error_f("%s:%zu:%zu: Expected: %s, but got %s: %.*s", t.loc.file_path, t.loc.row, t.loc.col,  lexer_kind_name(id), lexer_kind_name(t.id), sv_fmt(t.src));
    }
    return NULL;
}

Error* lexer_expect_cstr(lexer_token t, const char* cstr)
{
    if (!sv_equal_cstr(t.src, cstr)) {
        return error_f("%s:%zu:%zu: %s expected but got: %.*s", t.loc.file_path, t.loc.row, t.loc.col, cstr, sv_fmt(t.src));
    }
    return NULL;
}

bool chop_char(lexer *l)
{
    if (l->cur < l->content.count) {
        char x = l->content.data[l->cur];
        l->cur++;
        if (x == '\n') {
            l->bol = l->cur;
            l->row += 1;
        }
        return true;
    }
    return false;
}

void chop_chars(lexer *l, size_t n)
{
    while (n --> 0 && chop_char(l));
}

void trim_left(lexer *l)
{
    while (l->cur < l->content.count && isspace(l->content.data[l->cur])) {
        chop_char(l);
    }
}

void drop_until_endline(lexer *l)
{
    while (l->cur < l->content.count) {
        char x = l->content.data[l->cur];
        chop_char(l);
        if (x == '\n') break;
    }
}

void chop_until_prefix(lexer *l, const char *prefix)
{
    string_view content = sv_from_parts(l->content.data + l->cur, l->content.count);
    while (l->cur < l->content.count && !sv_start_with(content, prefix)) {
        content = sv_from_parts(l->content.data + l->cur, l->content.count);
        chop_char(l);
    }
}

location create_location(lexer *l)
{
    return (location) {
        .file_path = l->file_path,
        .row = l->row + 1,
        .col = l->cur - l->bol + 1,
    };
}

bool is_symbol(char c)
{
    return isalpha(c) || c == '_';
}

const char* lexer_kind_name(lexer_kind k)
{
    return lexer_kind_names[k];
}

bool lexer_get_token(lexer* l, lexer_token* t)
{
    string_view content = sv_from_parts(l->content.data + l->cur, l->content.count);

another_trim_round:
    while (l->cur < l->content.count) {
        trim_left(l);
        content = sv_from_parts(l->content.data + l->cur, l->content.count);

        // Single line comment
        for (size_t i = 0; i < l->sl_comments_count; ++i) {
            if (sv_start_with(content, l->sl_comments[i])) {
                drop_until_endline(l);
                goto another_trim_round;
            }
        }

        // Multi line comment
        for (size_t i = 0; i < l->ml_comments_count; ++i) {
            const char *opening = l->ml_comments[i].opening;
            const char *closing = l->ml_comments[i].closing;
            if (sv_start_with(content, opening)) {
                chop_chars(l, strlen(opening));
                chop_until_prefix(l, closing);
                chop_chars(l, strlen(closing));
                goto another_trim_round;
            }
        }

        break;
    }
    
    memset(t, 0, sizeof(*t));

    t->loc = create_location(l);

    if (l->cur >= l->content.count) {
        t->id = LEXER_END;
        return false;
    }

    // Puncts
    for (int i = 0; i < l->puncts_count; i++) {
        if (sv_start_with(content, l->puncts[i])) {
            t->id = LEXER_PUNCT;
            size_t n = strlen(l->puncts[i]);
            t->src = sv_from_parts(content.data, n);
            chop_chars(l, n);
            return true;
        }
    }

    // Int
    if (isdigit(l->content.data[l->cur])) {
        size_t n = 0;
        while (l->cur < l->content.count && isdigit(l->content.data[l->cur])) {
            t->int_value = t->int_value*10 + l->content.data[l->cur] - '0';
            n += 1;
            chop_char(l);
            if (is_symbol(l->content.data[l->cur])) {
                return false;
            }
        }

        t->id = LEXER_INT;
        t->src = sv_from_parts(content.data, n);
        
        return true;
    }

    // Symbol
    if (is_symbol(l->content.data[l->cur])) {
        t->id = LEXER_SYMBOL;
        size_t n = 0;
        size_t begin = l->cur;
        while (l->cur < l->content.count && (is_symbol(l->content.data[l->cur]) || isdigit(l->content.data[l->cur]))) {
            n += 1;
            chop_char(l);
        }
        t->src = sv_from_parts(l->content.data + begin, n);

        // Keyword
        for (int i = 0; i < l->keywords_count; i++) {
            if (sv_equal_cstr(t->src, l->keywords[i])) {
                t->id = LEXER_KEYWORD;
                break;
            }
        }

        return true;
    }

    chop_char(l);
    t->src = sv_from_parts(l->content.data + l->cur - 1, 1);
    return false;
}

#endif // LEXER_IMPLEMENTATION
