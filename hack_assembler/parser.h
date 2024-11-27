#include <stddef.h>

#ifndef STRING_IMPLEMENTATION
    #define STRING_IMPLEMENTATION
#endif // STRING_IMPLEMENTATION
#include "../dynamic_array/string.h"
#ifndef SYMBOL_TABLE_IMPLEMENTATION
    #define SYMBOL_TABLE_IMPLEMENTATION
#endif // SYMBOL_TABLE_IMPLEMENTATION
#include "symbol_table.h"
#ifndef HACK_ASM_CONVERT_IMPLEMENTATION
    #define HACK_ASM_CONVERT_IMPLEMENTATION
#endif // HACK_ASM_CONVERT_IMPLEMENTATION
#include "convert.h"

typedef struct {
    const char* filename;
    string_builder* data;
    string_builder* out;
    symbol_table* st;
    bool is_hex;
} parser;

parser parser_init(const char* filename, string_builder *data);
void parser_destroy(parser p);
string_view parse(parser p);

#ifdef HACK_ASM_PARSER_IMPLEMENTATION
#include <stdio.h>

#define UTILS_IMPLEMENTATION
#include "../utils.h"

typedef enum {
    UNKNOWN_COMMAND = 0,
    A_COMMAND       = 1,
    C_COMMAND       = 2,
    L_COMMAND       = 3,
} command_type;

command_type parse_command_type(string_view command)
{
    if (command.data[0] == '@') {
        return A_COMMAND;
    }
    else if (command.data[0] == '(' && command.data[command.count - 1] == ')') {
        return L_COMMAND;
    }
    else {
        return C_COMMAND;
    }
}

string_view parse_symbol(parser p, string_view command)
{
    string_view symbol;
    symbol_table_item* symbol_item;

    if (command.data[0] == '@') {
        symbol = sv_from_cstr(command.data + 1);
        symbol = sv_split_cstr(&symbol, "\r\n");

        if (sv_isnumeric(symbol)) {
            return symbol;
        } else if ((symbol_item = st_find(p.st, symbol)) != NULL) {
            return sv_from_digit(symbol_item->value);
        } else {
            string_view sv = sv_from_digit(st_put_variable(p.st, symbol));
            return sv_from_digit(st_put_variable(p.st, symbol));
        }
    } else {
        string_view sv = sv_from_parts(command.data + 1, command.count - 2);
        return sv_from_parts(command.data + 1, command.count - 2);
    }
}

string_view parse_dest(string_view command)
{
    if (sv_in_c(command, '=')) {
        return sv_split_mul_c(&command, '=', 1);
    } else {
        return sv_from_cstr("");
    }
}

string_view parse_comp(string_view command)
{
    if (sv_in_c(command, '=')) {
        return sv_split_mul_c(&command, '=', 2);
    } else if (sv_in_c(command, ';')) {
        return sv_split_mul_c(&command, ';', 1);
    } else {
        return sv_from_cstr("");
    }
}

string_view parse_jump(string_view command)
{
    if (sv_in_c(command, ';')) {
        return sv_split_mul_c(&command, ';', 2);
    } else {
        return sv_from_cstr("");
    }
}

string_view parse(parser p)
{
    int counter = 0;

    string_view lines = sb_to_sv(p.data);

    string_view command;

    while (lines.count > 0) {
        string_view instr = sv_from_cstr("");
        counter += 1;

        command = sv_split_cstr(&lines, "\r\n");
        command = sv_trim_left(command);

        if (sv_start_with(command, "//") || command.count < 1) {
            continue;
        };

        command_type type = parse_command_type(command);
        char* hex;

        if (type == A_COMMAND) {
            string_view symbol = parse_symbol(p, command);

            if (p.is_hex) {
                sb_add_cstr(p.out, (hex = bin_to_hex(decimal_to_bin(sv_to_digit(symbol), 16), 16)));
                sb_add_cstr(p.out, " ");
                free(hex);
            } else {
                sb_add_cstr(p.out, decimal_to_bin(sv_to_digit(symbol), 16));
                sb_add_cstr(p.out, "\n");
            }
        } else if (type == C_COMMAND) {
            string_view dest = parse_dest(command);
            string_view comp = parse_comp(command);
            string_view jump = parse_jump(command);

            if (p.is_hex) {
                string_builder* sb = sb_init(NULL);
                sb_add_cstr(sb, "111");
                sb_add_cstr(sb, comp_str2bin(comp));
                sb_add_cstr(sb, dest_str2bin(dest));
                sb_add_cstr(sb, jump_str2bin(jump));
                sb_add_cstr(p.out, (hex = bin_to_hex(sb_to_sv(sb).data, 16)));
                sb_add_cstr(p.out, " ");
                free(hex);
                sb_free(sb);
            } else {
                sb_add_cstr(p.out, "111");
                sb_add_cstr(p.out, comp_str2bin(comp));
                sb_add_cstr(p.out, dest_str2bin(dest));
                sb_add_cstr(p.out, jump_str2bin(jump));
                sb_add_cstr(p.out, "\n");
            }
        } else if (type == UNKNOWN_COMMAND) {
            fprintf(stderr, "%s:%d: Unknown command: %.*s\n", p.filename, counter, (int)command.count, command.data);
        }
    }

    return sb_to_sv(p.out);
}

parser parser_init(const char* filename, string_builder *data)
{
    parser p = {0};

    p.filename = filename;
    p.data = data;
    p.out = sb_init(NULL);
    p.st = st_init();

    string_view lines = sb_to_sv(p.data);

    string_view command = sv_split_cstr(&lines, "\r\n");
    
    size_t curr_address = 0;

    while (lines.count > 0) {
        string_view instr = sv_from_cstr("");

        if (sv_start_with(command, "//") || command.count < 1) {
            command = sv_split_cstr(&lines, "\r\n");
            continue;
        };
        
        command_type type = parse_command_type(command);

        if (type == L_COMMAND) {
            string_view symbol = parse_symbol(p, command);
            st_put_label(p.st, symbol, curr_address);
        } else {
            curr_address += 1;
        }

        command = sv_split_cstr(&lines, "\r\n");
    }

    return p;
}

void parser_destroy(parser p)
{
    st_destroy(p.st);
    sb_free(p.out);
    sb_free(p.data);
}
#endif // HACK_ASM_PARSER_IMPLEMENTATION