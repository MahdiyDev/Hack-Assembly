#include <stdbool.h>
#include <stdio.h>

#define STRING_IMPLEMENTATION
#include "../dynamic_array/string.h"

#ifndef UTILS_IMPLEMENTATION
    #define UTILS_IMPLEMENTATION
#endif // UTILS_IMPLEMENTATION
#include "../utils.h"

#define LEXER_IMPLEMENTATION
#include "lexer.h"

#include "error.h"

#define return_defer(value) do { result = (value); goto defer; } while(0)

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

const char *puncts[] = { "+", "*", "(", ")", "{", "}", "==", "=", ";", "," };

const char* op_table[][2] = {
    { "+", "ADD" },
    { "-", "SUB" },
    { "&", "AND" },
    { "|", "OR" }, 
    { "<", "LT" },
    { ">", "GT" },
    { "=", "EQ" },
};

Error* compile_parameter_list(lexer* l, lexer_token* t)
{
    Error* error = NULL;
    string_view type = {0};

    lexer_get_token(l, t); // get ")" punct or symbol
    while (!sv_equal_c(t->src, ')')) {
        type = t->src;

        lexer_get_token(l, t); // get identifier name
        if (has_error(lexer_expect_id(l, *t, LEXER_SYMBOL))) return trace(error);

        // define parameters
        printf("Parameter: (%.*s) %.*s\n", sv_fmt(type), sv_fmt(t->src));

        lexer_get_token(l, t); // get "," or ")" punct

        if (sv_equal_c(t->src, ',')) {
            lexer_get_token(l, t); // get ")" punct or symbol
        } else {
            return lexer_expect_cstr(*t, ")");
        }
    }

    return NULL;
}

Error* compile_var_dec(lexer* l, lexer_token* t)
{
    Error* error = NULL;

    lexer_get_token(l, t); // get variable type
    string_view type = t->src;

    lexer_get_token(l, t); // get variable name
    if (has_error(lexer_expect_id(l, *t, LEXER_SYMBOL))) return trace(error);
    // define variable
    printf("Variable: (%.*s) %.*s\n", sv_fmt(type), sv_fmt(t->src));

    lexer_get_token(l, t); // get ";" punct

    while (!sv_equal_c(t->src, ';')) {
        if (sv_equal_c(t->src, ',')) {
            lexer_get_token(l, t); // get ";" punct or variable
            continue;
        }
        if (t->id != LEXER_SYMBOL) {
            return lexer_expect_cstr(*t, ";");
        }

        // define variable
        printf("Variable: (%.*s) %.*s\n", sv_fmt(type), sv_fmt(t->src));

        lexer_get_token(l, t); // get ";" punct or variable
    }

    if (has_error(lexer_expect_cstr(*t, ";"))) return trace(error);

    lexer_get_token(l, t); // get next token
    return NULL;
}

Error* compile_term(lexer* l, lexer_token* t)
{
    lexer_get_token(l, t); // get term

    if (t->id == LEXER_INT) {
        // write_push_pop('push', 'CONST', int(tk.curr_token))
        printf("Term: (INT) %.*s\n", sv_fmt(t->src));
    }

    return NULL;
}

Error* compile_expression(lexer* l, lexer_token* t)
{
    Error* error = NULL;

    if (has_error(compile_term(l, t))) return trace(error);

    const char* expressions[] = { "+", "-", "*", "/", "&", "|", "<", ">", "=" };

    while (sv_in_carr(t->src, expressions)) {
        string_view op = t->src;

        if (has_error(compile_term(l, t))) return trace(error);
        
        if (sv_in_ctable(op, op_table)) {
            // write_arithmetic(self.op_table.get(op))
        } else if (sv_equal_c(t->src, '*')) {
            // write_call('Math.multiply', 2)
        } else if (sv_equal_c(t->src, '/')) {
            // write_call('Math.divide', 2)
        } else {
            return error_f("%.*s not supported op.", sv_fmt(op));
        }
    }

    return NULL;
}

Error* compile_if_statement(lexer* l, lexer_token* t)
{
    Error* error = NULL;

    lexer_get_token(l, t); // get "(" punct
    if (has_error(lexer_expect_cstr(*t, "("))) return trace(error);

    if (has_error(compile_expression(l, t))) return trace(error);

    lexer_get_token(l, t); // get ")" punct
    if (has_error(lexer_expect_cstr(*t, ")"))) return trace(error);

    return NULL;
}

Error* compile_let_statement(lexer* l, lexer_token* t)
{
    lexer_get_token(l, t); // get "let" identifier

    if (t->id != LEXER_SYMBOL) {
        return error_f("%.*s is not a valid Jack identifer.", sv_fmt(t->src));
    }

    return NULL;
}

Error* compile_do_statement(lexer* l, lexer_token* t)
{
    //
    return NULL;
}

Error* compile_while_statement(lexer* l, lexer_token* t)
{
    //
    return NULL;
}

Error* compile_return_statement(lexer* l, lexer_token* t)
{
    //
    return NULL;
}

Error* compile_statements(lexer* l, lexer_token* t)
{
    Error* error = NULL;

    const char* statements[] = { "if", "while", "let", "do", "return" };
    while (sv_in_carr(t->src, statements)) {
        if (sv_equal_cstr(t->src, "if")) {
            error = compile_if_statement(l, t);
        } else if (sv_equal_cstr(t->src, "let")) {
            error = compile_let_statement(l, t);
        } else if (sv_equal_cstr(t->src, "do")) {
            error = compile_do_statement(l, t);
        } else if (sv_equal_cstr(t->src, "while")) {
            error = compile_while_statement(l, t);
        } else if (sv_equal_cstr(t->src, "return")) {
            error = compile_return_statement(l, t);
        }

        if (error != NULL) return trace(error);
    }

    return NULL;
}

Error* compile_subroutine(string_view class_name, lexer* l, lexer_token* t)
{
    Error* result = NULL;
    Error* error = NULL;

    string_view subroutine_type = t->src;

    if (sv_equal_cstr(subroutine_type, "method")) {
        // symbol_table.define('this', self.class_name, 'ARG')
    }

    lexer_get_token(l, t); // get function type
    lexer_get_token(l, t); // get function name
    if (has_error(lexer_expect_id(l, *t, LEXER_SYMBOL))) return_defer(trace(error));

    string_builder* func_name = sb_init(NULL);
    sb_add_f(func_name, "%.*s.%.*s", sv_fmt(class_name), sv_fmt(t->src));

    lexer_get_token(l, t); // get "(" punct
    if (has_error(lexer_expect_cstr(*t, "("))) return_defer(trace(error));

    if (has_error(compile_parameter_list(l, t))) return_defer(trace(error));

    lexer_get_token(l, t); // get "{" punct
    if (has_error(lexer_expect_cstr(*t, "{"))) return_defer(trace(error));

    lexer_get_token(l, t); // get variable keyword
    while (sv_equal_cstr(t->src, "var")) {
        if (has_error(compile_var_dec(l, t))) return_defer(trace(error));
    }

    // n_args = symbol_table.var_count('VAR')
    // write_function(func_name, n_args)

    if (sv_equal_cstr(subroutine_type, "constructor")) {
        // n_fields = symbol_table.var_count('FIELD')
        // write_push_pop('push', 'CONST', n_fields)
        // write_call('Memory.alloc', 1)
        // write_push_pop('pop', 'POINTER', 0)
    } else if (sv_equal_cstr(subroutine_type, "method")) {
        // write_push_pop('push', 'ARG', 0)
        // write_push_pop('pop', 'POINTER', 0)
    }

    if (has_error(compile_statements(l, t))) return_defer(trace(error));

defer:
    sb_free(func_name);
    return error;
}

Error* compile_class(lexer* l, lexer_token* t)
{
    Error* error = NULL;

    string_view class_name = {0};

    lexer_get_token(l, t); // get class name
    class_name = t->src;
    
    lexer_get_token(l, t); // get "{" punct
    if (has_error(lexer_expect_cstr(*t, "{"))) return trace(error);

    lexer_get_token(l, t); // get keyword
    if (has_error(lexer_expect_id(l, *t, LEXER_KEYWORD))) return trace(error);

    const char* f[] = { "constructor", "function", "method" };

    while (sv_in_carr(t->src, f)) {
        if (has_error(compile_subroutine(class_name, l, t))) return trace(error);
    }

    return NULL;
}

int main()
{
    Error* error = NULL;

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

    lexer_token t = {0};
    // printf("%s:%zu:%zu: [%s] %.*s\n", t.loc.file_path, t.loc.row, t.loc.col, lexer_kind_name(t.id), (int)t.src.count, t.src.data);
    if (lexer_get_token(&l, &t)) {
        if (sv_equal_cstr(t.src, "class")) {
            if (has_error(compile_class(&l, &t))) {
                print_error(error);
            }
        } else {
            printf("ERROR %s:%zu:%zu: class definition is required\n", t.loc.file_path, t.loc.row, t.loc.col);
        }
    }
    // lexer_expect_id(&l, t, LEXER_END);

    sb_free(sb);

    return 0;
}