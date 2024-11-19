#include <stdbool.h>
#include <stdio.h>

#define UTILS_IMPLEMENTATION
#include "utils.h"
#define CONVERT_IMPLEMENTATION
#include "convert.h"
#define HACK_API_IMPLEMENTATION
#include "../hack_assembler/hack_api.h"

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

typedef struct {
    const char* filename;
    size_t current_idx;
    string_builder* data;
} parser;

command_type parse_command_type(string_view command)
{
    const char* commands[] = { "add", "sub", "and", "or", "lt", "gt", "eq", "neg", "not" };
    string_view token = sv_trim(command);
    token = sv_split(&token, ' ');

    if (sv_equal(token, sv_from_cstr("push"))) {
        return PUSH;
    }
    else if (sv_equal(token, sv_from_cstr("pop"))) {
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
        string_view result;
        for (int i = 0; i < 2; i++) {
            result = sv_split(&command, ' ');
            result = sv_split(&result, ' ');
        }
        return result;
    }
}

string_view command_arg2(string_view command)
{
    command_type types[] = { PUSH, POP };

    if (command_type_arr_in(parse_command_type(command), types, arr_count(types))) {
        string_view result;
        for (int i = 0; i < 3; i++) {
            result = sv_split(&command, ' ');
            result = sv_split(&result, ' ');
        }
        return result;
    } else {
        fprintf(stderr, "Argument2 does not exist\n");
        return sv_from_cstr("");
    }
}

string_view parse(parser p)
{
    int counter = 0;
    string_builder* out = sb_init("");

    string_builder* lines = sb_init("");
    read_file(p.filename, lines);
    string_view lines_sv = sb_to_sv(lines);

    string_view command = sv_split(&lines_sv, '\n');
    command = sv_split(&command, '\r'); // for windows

    string_view instr = sv_from_cstr("");
    while (command.count > 2) {
        counter += 1;

        if (sv_start_with(command, "//")) {
            command = sv_split(&lines_sv, '\n');
            command = sv_split(&command, '\r'); // for windows
            continue;
        };

        command_type type = parse_command_type(command);
        sb_add_str(out, sb_sprintf("// %.*s\n", command.count, command.data));

        if (type == PUSH) {
            instr = push(command_arg1(command), command_arg2(command), p.filename);
        } else if (type == POP) {
            instr = pop(command_arg1(command), command_arg2(command), p.filename);
        } else if (type == ARITHMETIC) {
            instr = arithmetic(command, counter);
        } else {
            fprintf(stderr, "Unknown command: %.*s\n", (int)command.count, command.data);
        }

        if (instr.count > 2) sb_add_str(out, sb_sprintf("%.*s", instr.count, instr.data));
        
        command = sv_split(&lines_sv, '\n');
        command = sv_split(&command, '\r'); // for windows
    }

    return sb_to_sv(out);
}

parser parser_init(const char* filename)
{
    parser p = {0};

    p.current_idx = -1;
    p.filename = filename;
    da_init(p.data);

    return p;
}

void parser_destroy(parser p)
{
    da_free(p.data);
}

int main()
{
    parser p = parser_init("test.vm");

    hack_asm _asm = init_hack_asm();

    string_view parsed_data = parse(p);

    FILE* fp = fopen("test.asm", "w");

    // Default values
    fprintf(fp, "(INIT)\n");

    set_default(fp, "SP", "256");     // stack pointer
    set_default(fp, "LCL", "300");    // base address of the local segment
    set_default(fp, "ARG", "400");    // base address of the argument segment
    set_default(fp, "THIS", "3000");  // base address of the this segment
    set_default(fp, "THAT", "3010");  // base address of the that segment;

    fprintf(fp, "%s\n", parsed_data.data);

    // End program
    fprintf(fp, "@INIT\n");
    fprintf(fp, "0;JMP\n");

    fclose(fp);

    // parse_instructions(parsed_data.data, &_asm);

    // print_instruction(stdout, &_asm);

    destroy_hack_asm(&_asm);

    parser_destroy(p);
    return 0;
}