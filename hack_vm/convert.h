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

string_view push(string_view segment, string_view index, const char* filename);
string_view pop(string_view segment, string_view index, const char* filename);
string_view arithmetic(string_view operation, int counter);
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
        string_view map_key = sv_from_cstr(map[i].key);
        if (sv_equal(key, map_key)) {
            return sv_from_cstr(map[i].value);
        }
    }
    return sv_from_cstr("");
}

#define map_get_carr(map, key) map_get(map, arr_count(map), key)

string_view get_base_address_index(string_view segment, string_view index) {
    const char* segments[] = { "static", "temp", "internal" };

    if (sv_equal(segment, sv_from_cstr("constant"))) {
        return index;
    } else if (sv_equal(segment, sv_from_cstr("pointer"))) {
        return sv_equal(index, sv_from_cstr("0"))
            ? map_get_carr(segment_symbol, sv_from_cstr("this"))
            : map_get_carr(segment_symbol, sv_from_cstr("that"));
    } else if (sv_in_carr(segment, segments)) {
        string_view value = map_get_carr(segment_symbol, segment);

        if (!isdigit(value.data[0])) return sv_from_cstr("");
        string_builder* sb = sb_init(NULL);

        long sum = strtol(value.data, NULL, 10) + strtol(index.data, NULL, 10);
        
        sb_add_str(sb, sb_sprintf("%ld", sum));

        return sb_to_sv(sb);
    } else {
        return map_get_carr(segment_symbol, segment);
    }
}

string_view get_base_address(string_view segment) { return get_base_address_index(segment, sv_from_cstr("0")); }

char* get_addressf(string_view segment, string_view index, const char* filename) {
    char* address = NULL;

    if (!sv_equal(segment, sv_from_cstr("static"))) {
        string_view base_address = get_base_address_index(segment, index);
        address = sb_sprintf("@%.*s\n", base_address.count, base_address.data);
    } else {
        address = sb_sprintf("@%s.%.*s\n", filename, index.count, index.data);
    }

    return address;
}

string_view push(string_view segment, string_view index, const char* filename) {
    const char* segments[] = { "pointer", "static", "temp", "internal" };

    string_view segment_type;
    string_builder* sb = sb_init(NULL);

    if (sv_equal(segment, sv_from_cstr("constant"))) {
        sb_add_str(sb, "D=A\n");

        segment_type = sv_from_cstr("constant");
    } else if (sv_in_carr(segment, segments)) {
        sb_add_str(sb, "D=A\n");

        segment_type = segment;
    } else { 
        sb_add_str(sb, "D=M\n");
        sb_add_str(sb, sb_sprintf("@%.*s\n", index.count, index.data));
        sb_add_str(sb, "A=D+A\n");
        sb_add_str(sb, "D=M\n");

        segment_type = segment;
    }

    char* address = get_addressf(segment, index, filename);

    string_builder* sb_result = sb_init(NULL);

    sb_add_str(sb_result, address);
    sb_add_str(sb_result, sb_to_sv(sb).data);
    sb_add_str(sb_result, "@SP\n");
    sb_add_str(sb_result, "M=M+1\n");
    sb_add_str(sb_result, "A=M-1\n");
    sb_add_str(sb_result, "M=D\n");

    sb_free(sb);

    return sb_to_sv(sb_result);
}

string_view pop(string_view segment, string_view index, const char* filename) {
    const char* segments[] = { "pointer", "static", "temp", "internal" };

    string_builder* sb = sb_init(NULL);

    char* address = get_addressf(segment, index, filename);

    if (sv_equal(segment, sv_from_cstr("constant"))) {
        fprintf(stderr, "Invalid pop into constant.\n");
        sb_free(sb);
        exit(EXIT_FAILURE);
    } else if (sv_in_carr(segment, segments)) {
        sb_add_str(sb, "@SP\n");
        sb_add_str(sb, "AM=M-1\n");
        sb_add_str(sb, "D=M\n");
        sb_add_str(sb, address);
        sb_add_str(sb, "M=D\n");
        

        return sb_to_sv(sb);
    } else {
        sb_add_str(sb, sb_sprintf("@%s\n", get_base_address(segment).data));
        sb_add_str(sb, "D=M\n");
        sb_add_str(sb, sb_sprintf("@%.*s\n", index.count, index.data));
        sb_add_str(sb, "D=D+A\n");
        sb_add_str(sb, "@15\n");
        sb_add_str(sb, "M=D\n");
        sb_add_str(sb, "@SP\n");
        sb_add_str(sb, "AM=M-1\n");
        sb_add_str(sb, "D=M\n");
        sb_add_str(sb, "@15\n");
        sb_add_str(sb, "A=M\n");
        sb_add_str(sb, "M=D\n");

        return sb_to_sv(sb);
    }
}

string_view arithmetic(string_view operation, int counter) {
    const char* neg_not[] = { "neg", "not" };
    const char* arith[] = { "add", "sub", "and", "or" };

    string_builder* sb = sb_init("");

    sb_add_str(sb, pop(sv_from_cstr("internal"), sv_from_cstr("0"), "").data);

    string_view op = map_get_carr(operation_symbol, operation);

    if (sv_in_carr(operation, neg_not)) {
        sb_add_str(sb, "@13\n");
        sb_add_str(sb, sb_sprintf("D=%sM\n", op.data));
        sb_add_str(sb, "@15\n");
        sb_add_str(sb, "M=D\n");
    } else {
        sb_add_str(sb, pop(sv_from_cstr("internal"), sv_from_cstr("1"), "").data);

        if (sv_in_carr(operation, arith)) {
            sb_add_str(sb, "@13\n");
            sb_add_str(sb, "D=M\n");
            sb_add_str(sb, "@14\n");
            sb_add_str(sb, sb_sprintf("D=M%sD\n", op.data));
            sb_add_str(sb, "@15\n");
            sb_add_str(sb, "M=D\n");
        } else {
            sb_add_str(sb, "@13\n");
            sb_add_str(sb, "D=M\n");
            sb_add_str(sb, "@14\n");
            sb_add_str(sb, "D=M-D\n");
            sb_add_str(sb, sb_sprintf("@false%d\n", counter));
            sb_add_str(sb, sb_sprintf("D;%s", op.data));
            sb_add_str(sb, "D=-1\n");
            sb_add_str(sb, sb_sprintf("@set%d\n", counter));
            sb_add_str(sb, "0;JMP\n");
            sb_add_str(sb, sb_sprintf("(false%d)\n", counter));
            sb_add_str(sb, "D=0\n");
            sb_add_str(sb, sb_sprintf("(set%d)\n", counter));
            sb_add_str(sb, "@15\n");
            sb_add_str(sb, "M=D\n");
        }
    }

    sb_add_str(sb, push(sv_from_cstr("internal"), sv_from_cstr("2"), "").data);
    return sb_to_sv(sb);
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
