/*
Usage:
```c
int main()
{
    const char* filename = "main";
    instructions* instr = NULL;

    // Default values
    printf("(INIT)\n");

    set_default("SP", "256");     // stack pointer
    set_default("LCL", "300");    // base address of the local segment
    set_default("ARG", "400");    // base address of the argument segment
    set_default("THIS", "3000");  // base address of the this segment
    set_default("THAT", "3010");  // base address of the that segment;

    print_instr(push(sv_from_cstr("constant"), sv_from_cstr("21"), filename));
    print_instr(push(sv_from_cstr("pointer"), sv_from_cstr("10"), filename));
    print_instr(push(sv_from_cstr("static"), sv_from_cstr("10"), filename));
    print_instr(push(sv_from_cstr("local"), sv_from_cstr("INDEX"), filename));
    print_instr(arithmetic(sv_from_cstr("add"), 0));
    print_instr(pop (sv_from_cstr("static"), sv_from_cstr("11"), filename));
    print_instr(pop (sv_from_cstr("local"), sv_from_cstr("11"), filename));

    // End program
    printf("@INIT\n");
    printf("0;JMP\n");

    return 0;
}
```
*/
#pragma once
#include <stdio.h>

#ifndef STRING_IMPLEMENTATION
    #define STRING_IMPLEMENTATION
#endif
#include "../dynamic_array/string.h"

string_builder* push(string_view segment, string_view index, const char* filename);
string_builder* pop(string_view segment, string_view index, const char* filename);
string_builder* arithmetic(string_view operation, int counter);
void set_default(FILE* fp, const char* address, const char* value);

#ifdef CONVERT_IMPLEMENTATION
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    char** data;
    int count;
} instructions;

typedef struct {
    const char* key;
    const char* value;
} sybmol_map;

const sybmol_map segment_symbol[] = {
    { "local", "LCL" },
    { "argument", "ARG" },
    { "this", "THIS" },
    { "that", "THAT" },
    { "static", "16" },
    { "temp", "5" },
    { "internal", "13" },
};

const sybmol_map operation_symbol[] = {
    { "add", "+" },
    { "sub", "-" },
    { "and", "&" },
    { "or", "|" },
    { "neg", "-" },
    { "not", "!" },
    { "eq", "JNE" },
    { "gt", "JLE" },
    { "lt", "JGE" },
};

string_view map_get(const sybmol_map* map, int count, string_view key) {
    for (int i = 0; i < count; i++) {
        if (sv_equal_cstr(key, map[i].key)) {
            return sv_from_cstr(map[i].value);
        }
    }
    return sv_from_cstr("");
}

#define map_get_carr(map, key) map_get(map, arr_count(map), key)

string_view get_base_address_index(string_builder* address_sb, string_view segment, string_view index) {
    const char* segments[] = { "static", "temp", "internal" };

    if (sv_equal_cstr(segment, "constant")) {
        return index;
    } else if (sv_equal_cstr(segment, "pointer")) {
        return sv_equal_cstr(index, "0")
            ? map_get_carr(segment_symbol, sv_from_cstr("this"))
            : map_get_carr(segment_symbol, sv_from_cstr("that"));
    } else if (sv_in_carr(segment, segments)) {
        string_view value = map_get_carr(segment_symbol, segment);

        if (!isdigit(value.data[0])) return sv_from_cstr("");

        long sum = strtol(value.data, NULL, 10) + strtol(index.data, NULL, 10);
        sb_add_f(address_sb, "%ld", sum);
        return sb_to_sv(address_sb);
    } else {
        return map_get_carr(segment_symbol, segment);
    }
}

string_view get_base_address(string_builder* address_sb, string_view segment) { 
    return get_base_address_index(address_sb, segment, sv_from_cstr("0"));
}

void add_address(string_builder* sb, string_view segment, string_view index, const char* filename) {
    if (!sv_equal_cstr(segment, "static")) {
        string_builder* address_sb = sb_init(NULL);
        string_view base_address = get_base_address_index(address_sb, segment, index);
        sb_add_f(sb, "@%.*s\n", base_address.count, base_address.data);
        sb_free(address_sb);
    } else {
        sb_add_f(sb, "@%s.%.*s\n", filename, index.count, index.data);
    }
}

string_builder* push(string_view segment, string_view index, const char* filename) {
    const char* segments[] = { "pointer", "static", "temp", "internal" };

    string_view segment_type;
    string_builder* sb = sb_init("");

    if (sv_equal_cstr(segment, "constant")) {
        sb_add_cstr(sb, "D=A\n");

        segment_type = sv_from_cstr("constant");
    } else if (sv_in_carr(segment, segments)) {
        sb_add_cstr(sb, "D=M\n");

        segment_type = segment;
    } else { 
        sb_add_cstr(sb, "D=M\n");
        sb_add_f(sb, "@%.*s\n", index.count, index.data);
        sb_add_cstr(sb, "A=D+A\n");
        sb_add_cstr(sb, "D=M\n");

        segment_type = segment;
    }

    string_builder* sb_result = sb_init("");

    add_address(sb_result, segment, index, filename);
    sb_add_cstr(sb_result, sb_to_sv(sb).data);
    sb_add_cstr(sb_result, "@SP\n");
    sb_add_cstr(sb_result, "M=M+1\n");
    sb_add_cstr(sb_result, "A=M-1\n");
    sb_add_cstr(sb_result, "M=D\n");

    sb_free(sb);

    return sb_result;
}

string_builder* pop(string_view segment, string_view index, const char* filename) {
    const char* segments[] = { "pointer", "static", "temp", "internal" };

    string_builder* sb = sb_init("");


    if (sv_equal_cstr(segment, "constant")) {
        fprintf(stderr, "Invalid pop into constant.\n");
        sb_free(sb);
        exit(EXIT_FAILURE);
    } else if (sv_in_carr(segment, segments)) {
        sb_add_cstr(sb, "@SP\n");
        sb_add_cstr(sb, "AM=M-1\n");
        sb_add_cstr(sb, "D=M\n");
        add_address(sb, segment, index, filename);
        sb_add_cstr(sb, "M=D\n");
        

        return sb;
    } else {
        string_builder* address_sb = sb_init(NULL);

        sb_add_f(sb, "@%s\n", get_base_address(address_sb, segment).data);
        sb_add_cstr(sb, "D=M\n");
        sb_add_f(sb, "@%.*s\n", index.count, index.data);
        sb_add_cstr(sb, "D=D+A\n");
        sb_add_cstr(sb, "@15\n");
        sb_add_cstr(sb, "M=D\n");
        sb_add_cstr(sb, "@SP\n");
        sb_add_cstr(sb, "AM=M-1\n");
        sb_add_cstr(sb, "D=M\n");
        sb_add_cstr(sb, "@15\n");
        sb_add_cstr(sb, "A=M\n");
        sb_add_cstr(sb, "M=D\n");

        sb_free(address_sb);
        return sb;
    }
}

string_builder* arithmetic(string_view operation, int counter) {
    const char* neg_not[] = { "neg", "not" };
    const char* arith[] = { "add", "sub", "and", "or" };

    string_builder* sb = sb_init("");
    string_builder* instr = NULL;

    instr = pop(sv_from_cstr("internal"), sv_from_cstr("0"), "");
    sb_add(sb, sb_to_sv(instr));
    sb_free(instr);

    string_view op = map_get_carr(operation_symbol, operation);

    if (sv_in_carr(operation, neg_not)) {
        sb_add_cstr(sb, "@13\n");
        sb_add_f(sb, "D=%sM\n", op.data);
        sb_add_cstr(sb, "@15\n");
        sb_add_cstr(sb, "M=D\n");
    } else {
        instr = pop(sv_from_cstr("internal"), sv_from_cstr("1"), "");
        sb_add(sb, sb_to_sv(instr));
        sb_free(instr);

        if (sv_in_carr(operation, arith)) {
            sb_add_cstr(sb, "@13\n");
            sb_add_cstr(sb, "D=M\n");
            sb_add_cstr(sb, "@14\n");
            sb_add_f(sb, "D=D%sM\n", op.data);
            sb_add_cstr(sb, "@15\n");
            sb_add_cstr(sb, "M=D\n");
        } else {
            sb_add_cstr(sb, "@13\n");
            sb_add_cstr(sb, "D=M\n");
            sb_add_cstr(sb, "@14\n");
            sb_add_cstr(sb, "D=M-D\n");
            sb_add_f(sb, "@false%d\n", counter);
            sb_add_f(sb, "D;%s", op.data);
            sb_add_cstr(sb, "D=-1\n");
            sb_add_f(sb, "@set%d\n", counter);
            sb_add_cstr(sb, "0;JMP\n");
            sb_add_f(sb, "(false%d)\n", counter);
            sb_add_cstr(sb, "D=0\n");
            sb_add_f(sb, "(set%d)\n", counter);
            sb_add_cstr(sb, "@15\n");
            sb_add_cstr(sb, "M=D\n");
        }
    }

    instr = push(sv_from_cstr("internal"), sv_from_cstr("2"), "");
    sb_add(sb, sb_to_sv(instr));
    sb_free(instr);
    return sb;
}

void print_instr(string_view instr)
{
    printf("%s\n", instr.data);
}

void set_default(FILE* fp, const char* address, const char* value)
{
    fprintf(fp, "@%s\n", value);
    fprintf(fp, "D=A\n");
    fprintf(fp, "@%s\n", address);
    fprintf(fp, "M=D\n");
    fprintf(fp, "\n");
}
#endif // CONVERT_IMPLEMENTATION
