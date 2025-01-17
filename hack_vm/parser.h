#pragma once

#include <stdio.h>
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
    LABEL = 4,
    GOTO = 5,
    IF = 6,
    FUNCTION = 7,
    RETURN = 8,
    CALL = 9,
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
    else if (sv_equal_cstr(token, "label")) {
        return LABEL;
    }
    else if (sv_equal_cstr(token, "goto")) {
        return GOTO;
    }
    else if (sv_equal_cstr(token, "if-goto")) {
        return IF;
    }
    else if (sv_equal_cstr(token, "function")) {
        return FUNCTION;
    }
    else if (sv_equal_cstr(token, "call")) {
        return CALL;
    }
    else if (sv_equal_cstr(token, "return")) {
        return RETURN;
    }
    else {
        return UNKNOWN;
    }
}

string_view command_arg1(string_view command, command_type type, const char* filename, int counter)
{
    if (type == ARITHMETIC) return command;
    else if (type != RETURN) {
        return sv_split_mul_c(&command, ' ', 2);
    }
    else {
        fprintf(stderr, "%s.vm:%d: Argument1 does not exist\n", filename, counter);
        return sv_from_cstr("");
    }
}

string_view command_arg2(string_view command, command_type type, const char* filename, int counter)
{
    command_type types[] = { PUSH, POP, FUNCTION, CALL };

    if (command_type_arr_in(type, types, arr_count(types))) {
        return sv_split_mul_c(&command, ' ', 3);
    }
    else {
        fprintf(stderr, "%s.vm:%d: Argument2 does not exist\n", filename, counter);
        return sv_from_cstr("");
    }
}

string_view parse(parser p)
{
    int counter = 0;

    string_view lines = sb_to_sv(p.data);

    string_view command;

    while (lines.count > 0) {
        string_builder* instr = NULL;
        counter += 1;

        command = sv_split_cstr(&lines, "\r\n");
        command = sv_trim_left(command);

        if (sv_start_with(command, "//") || command.count < 1) {
            continue;
        }

        string_view arg1;
        string_view arg2;

        command_type type = parse_command_type(command);

        if (type == PUSH) {
            arg1 = command_arg1(command, type, p.filename, counter);
            arg2 = command_arg2(command, type, p.filename, counter);
            instr = write_push_sv(arg1, arg2, p.filename);
        }
        else if (type == POP) {
            arg1 = command_arg1(command, type, p.filename, counter);
            arg2 = command_arg2(command, type, p.filename, counter);
            instr = write_pop_sv(arg1, arg2, p.filename);
        }
        else if (type == ARITHMETIC) {
            instr = write_arithmetic(command, counter);
        }
        else if (type == LABEL) {
            arg1 = command_arg1(command, type, p.filename, counter);
            instr = write_label(arg1);
        }
        else if (type == GOTO) {
            arg1 = command_arg1(command, type, p.filename, counter);
            instr = write_goto_sv(arg1);
        }
        else if (type == IF) {
            arg1 = command_arg1(command, type, p.filename, counter);
            instr = write_if_goto(arg1);
        }
        else if (type == FUNCTION) {
            arg1 = command_arg1(command, type, p.filename, counter);
            arg2 = command_arg2(command, type, p.filename, counter);
            instr = write_function(arg1, arg2, p.filename, counter);
        }
        else if (type == CALL) {
            arg1 = command_arg1(command, type, p.filename, counter);
            arg2 = command_arg2(command, type, p.filename, counter);
            instr = write_call(arg1, arg2, p.filename, counter);
        }
        else if (type == RETURN) {
            instr = write_return();
        }
        else {
            fprintf(stderr, "%s.vm:%d: Unknown command: %.*s\n", p.filename, counter, (int)command.count, command.data);
        }

        if (instr != NULL) {
            if (instr->count > 1) {
                sb_add_f(p.out, "// %.*s\n", command.count, command.data);
                sb_add(p.out, sb_to_sv(instr));
            }

            sb_free(instr);
        };
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