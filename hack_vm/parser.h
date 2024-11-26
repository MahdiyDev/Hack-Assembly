#pragma once

#include <stdbool.h>
#include <stdlib.h>

#ifndef STRING_IMPLEMENTATION
    #define STRING_IMPLEMENTATION
#endif // STRING_IMPLEMENTATION
#include "../dynamic_array/string.h"

typedef struct {
    const char* filename;
    string_builder* data;
    string_builder* out;
} parser;

parser parser_init(const char* filename, string_builder* data);
void parser_destroy(parser p);
string_view parse(parser p);

#ifdef HACK_VM_PARSER_IMPLEMENTATION
#ifndef CONVERT_IMPLEMENTATION
    #define CONVERT_IMPLEMENTATION
#endif // CONVERT_IMPLEMENTATION
#include "convert.h"

typedef enum {
    UNKNOWN = 0,
    ARITHMETIC = 1,
    PUSH = 2,
    POP = 3,
} command_type;

bool command_type_arr_in(command_type type, command_type* arr, int count)
{
	for (int i = 0; i < count; i++) {
        if (arr[i] == type) return true;
    }
    return false;
}

command_type parse_command_type(string_view command)
{
    const char* commands[] = { "add", "sub", "and", "or", "lt", "gt", "eq", "neg", "not" };
    string_view token = sv_trim(command);
    token = sv_split_c(&token, ' ');

    if (sv_equal_cstr(token, "push")) {
        return PUSH;
    }
    else if (sv_equal_cstr(token, "pop")) {
        return POP;
    }
    else if (sv_in_carr(token, commands)) {
        return ARITHMETIC;
    }
    else {
        return UNKNOWN;
    }
}

string_view command_arg1(string_view command)
{
    if (parse_command_type(command) == ARITHMETIC) return command;
    else {
        return sv_split_mul_c(&command, ' ', 2);
    }
}

string_view command_arg2(string_view command)
{
    command_type types[] = { PUSH, POP };

    if (command_type_arr_in(parse_command_type(command), types, arr_count(types))) {
        return sv_split_mul_c(&command, ' ', 3);
    } else {
        fprintf(stderr, "Argument2 does not exist\n");
        return sv_from_cstr("");
    }
}

string_view parse(parser p)
{
    int counter = 0;

    string_view lines = sb_to_sv(p.data);

    string_view command = sv_split_cstr(&lines, "\r\n");

    while (lines.count > 0) {
        string_builder* instr = NULL;
        counter += 1;

        if (sv_start_with(command, "//") || command.count < 1) {
            command = sv_split_cstr(&lines, "\r\n");
            continue;
        };

        command_type type = parse_command_type(command);

        if (type == PUSH) {
            instr = push(command_arg1(command), command_arg2(command), p.filename);
        } else if (type == POP) {
            instr = pop(command_arg1(command), command_arg2(command), p.filename);
        } else if (type == ARITHMETIC) {
            instr = arithmetic(command, counter);
        } else {
            fprintf(stderr, "%s:%d: Unknown command: %.*s\n", p.filename, counter, (int)command.count, command.data);
        }

        if (instr != NULL && instr->count > 1) {
            sb_add_f(p.out, "// %.*s\n", command.count, command.data);
            sb_add_f(p.out, "%.*s", instr->count, instr->items);
        };
        
        sb_free(instr);

        command = sv_split_cstr(&lines, "\r\n");
    }

    return sb_to_sv(p.out);
}

parser parser_init(const char* filename, string_builder* data)
{
    parser p = {0};

    p.filename = filename;
    p.data = data;
    p.out = sb_init("");

    return p;
}

void parser_destroy(parser p)
{
    sb_free(p.out);
    sb_free(p.data);
}
#endif // HACK_VM_PARSER_IMPLEMENTATION