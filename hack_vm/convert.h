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

    print_instr(write_push("constant", "21", filename));
    print_instr(write_push("pointer", "10", filename));
    print_instr(write_push("static", "10", filename));
    print_instr(write_push("local", "INDEX", filename));
    print_instr(write_arithmetic("add", 0));
    print_instr(write_pop ("static", "11", filename));
    print_instr(write_pop ("local", "11", filename));

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

string_builder* write_push(const char* segment, const char* index, const char* filename);
string_builder* write_push_sv(string_view segment, string_view index, const char* filename);
string_builder* write_pop(const char* segment, const char* index, const char* filename);
string_builder* write_pop_sv(string_view segment, string_view index, const char* filename);
string_builder* write_arithmetic(string_view operation, int counter);
string_builder* write_label(string_view label);
string_builder* write_goto(const char* label);
string_builder* write_goto_sv(string_view label);
string_builder* write_if_goto(string_view label);
string_builder* write_function(string_view function_name, string_view args, const char* filename, int counter);
string_builder* write_call(string_view function_name, string_view args, const char* filename, int counter);
string_builder* write_return();

void set_default(FILE* fp, const char* address, const char* value);

#ifdef CONVERT_IMPLEMENTATION
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

string_builder* assign(const char* save_to, const char* save_from);
string_builder* assign_with_frame(const char* save_to, const char* save_from, int* frame_steps);
string_builder* assign_pointer_with_frame(const char* save_to, const char* save_from, int* frame_steps);

typedef struct {
    char** data;
    int count;
} instructions;

typedef struct {
    const char* key;
    const char* value;
} sybmol_map;

const sybmol_map segment_symbol[] = {
    { "local"   , "LCL" },
    { "argument", "ARG" },
    { "this"    , "THIS" },
    { "that"    , "THAT" },
    { "static"  , "16" },
    { "temp"    , "5" },
    { "internal", "13" },
};

const sybmol_map operation_symbol[] = {
    { "add", "+" },
    { "sub", "-" },
    { "and", "&" },
    { "or" , "|" },
    { "neg", "-" },
    { "not", "!" },
    { "eq" , "JNE" },
    { "gt" , "JLE" },
    { "lt" , "JGE" },
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
        string_view value = map_get_carr(segment_symbol, segment);
        if (sv_equal_cstr(value, "")) return segment; // R1, R2, R3 ... R15
        return value;
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

string_builder* write_push(const char* segment, const char* index, const char* filename)
{
    return write_push_sv(sv_from_cstr(segment), sv_from_cstr(index), filename);
}

string_builder* write_push_sv(string_view segment, string_view index, const char* filename)
{
    const char* segments[] = { "pointer", "static", "temp", "internal" };

    string_builder* sb = sb_init("");

    if (sv_equal_cstr(segment, "constant")) {
        sb_add_cstr(sb, "D=A\n");
    } else if (sv_in_carr(segment, segments)) {
        sb_add_cstr(sb, "D=M\n");
    } else { 
        sb_add_cstr(sb, "D=M\n");
        sb_add_f(sb, "@%.*s\n", index.count, index.data);
        sb_add_cstr(sb, "A=D+A\n");
        sb_add_cstr(sb, "D=M\n");
    }

    string_builder* sb_result = sb_init("");

    add_address(sb_result, segment, index, filename);
    sb_add(sb_result, sb_to_sv(sb));
    sb_add_cstr(sb_result, "@SP\n");
    sb_add_cstr(sb_result, "M=M+1\n");
    sb_add_cstr(sb_result, "A=M-1\n");
    sb_add_cstr(sb_result, "M=D\n");

    sb_free(sb);

    return sb_result;
}

string_builder* write_pop(const char* segment, const char* index, const char* filename)
{
    return write_pop_sv(sv_from_cstr(segment), sv_from_cstr(index), filename);
}

string_builder* write_pop_sv(string_view segment, string_view index, const char* filename)
{
    const char* segments[] = { "pointer", "static", "temp", "internal" };

    string_builder* sb = sb_init("");

    if (sv_equal_cstr(segment, "constant")) {
        fprintf(stderr, "Invalid write_pop into constant.\n");
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

string_builder* write_arithmetic(string_view operation, int counter) {
    const char* neg_not[] = { "neg", "not" };
    const char* arith[] = { "add", "sub", "and", "or" };

    string_builder* sb = sb_init("");
    string_builder* instr = NULL;

    instr = write_pop("internal", "0", "");
    sb_add(sb, sb_to_sv(instr));
    sb_free(instr);

    string_view op = map_get_carr(operation_symbol, operation);

    if (sv_in_carr(operation, neg_not)) {
        sb_add_cstr(sb, "@13\n");
        sb_add_f(sb, "D=%sM\n", op.data);
        sb_add_cstr(sb, "@15\n");
        sb_add_cstr(sb, "M=D\n");
    } else {
        instr = write_pop("internal", "1", "");
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

    instr = write_push("internal", "2", "");
    sb_add(sb, sb_to_sv(instr));
    sb_free(instr);
    return sb;
}

string_builder* write_function(string_view function_name, string_view args, const char* filename, int counter)
{
    if (!sv_isnumeric(args)) {
        fprintf(stderr, "%s.vm:%d: function argument count must be a number\n", filename, counter);
        return NULL;
    }
    size_t args_count = sv_to_digit(args);

    string_builder* sb = sb_init(NULL);
    string_builder* instr = NULL;
    
    sb_add_f(sb, "(%.*s)\n", function_name.count, function_name.data);

    for (int i = 0; i < args_count; i++) {
        instr = write_push("constant", "0", "");
        sb_add(sb, sb_to_sv(instr));
        sb_free(instr);
    }

    return sb;
}

string_builder* write_call(string_view function_name, string_view args, const char* filename, int counter)
{
    if (!sv_isnumeric(args)) {
        fprintf(stderr, "%s.vm:%d: function argument count must be a number\n", filename, counter);
        return NULL;
    }
    size_t args_count = sv_to_digit(args);

    string_builder* instr = NULL;
    string_builder* sb = sb_init(NULL);

    // write_push return-address
    string_builder* return_address_label = sb_init(NULL);

    sb_add_f(return_address_label, "%.*s$return.%d", function_name.count, function_name.data, counter);
    sb_add_c(return_address_label, '\0');
    instr = write_push("constant", sb_to_sv(return_address_label).data, "");
    sb_add(sb, sb_to_sv(instr));
    sb_free(instr);
    
    // save LCL of calling function
    instr = write_push("R1", "0", "");
    sb_add(sb, sb_to_sv(instr));
    sb_free(instr);

    // save ARG of calling function
    instr = write_push("R2", "0", "");
    sb_add(sb, sb_to_sv(instr));
    sb_free(instr);

    // save THIS of calling function
    instr = write_push("R3", "0", "");
    sb_add(sb, sb_to_sv(instr));
    sb_free(instr);

    // save THAT of calling function
    instr = write_push("R4", "0", "");
    sb_add(sb, sb_to_sv(instr));
    sb_free(instr);

    // reposition ARG (n=number of args)
    int steps_back = 0 - 5 - args_count;
    instr = assign_with_frame("ARG", "SP", &steps_back);
    sb_add(sb, sb_to_sv(instr));
    sb_free(instr);

    // reposition LCL
    instr = assign_with_frame("LCL", "SP", (int[]){0});
    sb_add(sb, sb_to_sv(instr));
    sb_free(instr);

    // transfer control
    instr = write_goto_sv(function_name);
    sb_add(sb, sb_to_sv(instr));
    sb_free(instr);

    // label for return address
    instr = write_label(sb_to_sv(return_address_label));
    sb_add(sb, sb_to_sv(instr));
    sb_free(instr);

    sb_free(return_address_label);

    return sb;
}

string_builder* write_return()
{
    string_builder* instr = NULL;
    string_builder* sb = sb_init(NULL);

    // save endFrame address as temp variable
    instr = assign("R14", "LCL");
    sb_add(sb, sb_to_sv(instr));
    sb_free(instr);

    // save return address as another temp variable
    instr = assign_with_frame("R15", "R14", (int[]){-5});
    sb_add(sb, sb_to_sv(instr));
    sb_free(instr);

    // Reposition return value for caller
    instr = write_pop("ARG", "0", "");
    sb_add(sb, sb_to_sv(instr));
    sb_free(instr);

    // Reposition SP of caller
    instr = assign_with_frame("SP", "ARG", (int[]){1});
    sb_add(sb, sb_to_sv(instr));
    sb_free(instr);

    // Restore THAT of caller
    instr = assign_pointer_with_frame("THAT", "R14", (int[]){-1});
    sb_add(sb, sb_to_sv(instr));
    sb_free(instr);

    // Restore THIS of caller
    instr = assign_pointer_with_frame("THIS", "R14", (int[]){-2});
    sb_add(sb, sb_to_sv(instr));
    sb_free(instr);

    // Restore ARG of caller
    instr = assign_pointer_with_frame("ARG", "R14", (int[]){-3});
    sb_add(sb, sb_to_sv(instr));
    sb_free(instr);

    // Restore LCL of caller
    instr = assign_pointer_with_frame("LCL", "R14", (int[]){-4});
    sb_add(sb, sb_to_sv(instr));
    sb_free(instr);

    // Jump to return address in caller
    instr = write_goto("R15");
    sb_add(sb, sb_to_sv(instr));
    sb_free(instr);

    return sb;
}

string_builder* write_label(string_view label)
{
    string_builder* sb = sb_init(NULL);
    sb_add_f(sb, "(%.*s)\n", label.count, label.data);
    return sb;
}

string_builder* write_goto(const char *label)
{
    return write_goto_sv(sv_from_cstr(label));
}

string_builder* write_goto_sv(string_view label)
{
    string_builder* sb = sb_init(NULL);
    sb_add_f(sb, "@%.*s\n", label.count, label.data);
    sb_add_cstr(sb, "0;JMP\n");
    return sb;
}

string_builder* write_if_goto(string_view label)
{
    string_builder* sb = sb_init(NULL);

    string_builder* instr = write_pop("internal", "2", "");
    sb_add(sb, sb_to_sv(instr));
    sb_add_cstr(sb, "@15\n");
    sb_add_cstr(sb, "D=M\n");
    sb_add_f(sb, "@%.*s\n", label.count, label.data);
    sb_add_cstr(sb, "D;JNE\n");
    sb_add_cstr(sb, "0;JMP\n");

    sb_free(instr);
    return sb;
}

string_builder* assign(const char* save_to, const char* save_from)
{
    return assign_with_frame(save_to, save_from, NULL);
}

string_builder* assign_with_frame(const char* save_to, const char* save_from, int* frame_steps)
{
    string_builder* sb = sb_init(NULL);
    sb_add_f(sb, "@%s\n", save_from);
    sb_add_cstr(sb, "D=M\n");

    char op;
    if (frame_steps != NULL) {
        int step = *frame_steps;
        if (step < 0) {
            op = '-';
            step = abs(step);
        }
        else {
            op = '+';
        }

        sb_add_f(sb, "@%d\n", step);
        sb_add_f(sb, "D=D%cA\n", op);
    }

    sb_add_f(sb, "@%s\n", save_to);
    sb_add_cstr(sb, "M=D\n");

    return sb;
}

string_builder* assign_pointer_with_frame(const char* save_to, const char* save_from, int* frame_steps)
{
    string_builder* sb = sb_init(NULL);
    sb_add_f(sb, "@%s\n", save_from);

    char op;
    if (frame_steps != NULL) {
        int step = *frame_steps;
        if (step < 0) {
            op = '-';
            step = abs(step);
        }
        else {
            op = '+';
        }

        sb_add_cstr(sb, "D=M\n");
        sb_add_f(sb, "@%d\n", step);
        sb_add_f(sb, "A=D%cA\n", op);
        sb_add_cstr(sb, "D=M\n");
    }
    else {
        sb_add_cstr(sb, "A=M\n");
        sb_add_cstr(sb, "D=M\n");
    }

    sb_add_f(sb, "@%s\n", save_to);
    sb_add_cstr(sb, "M=D\n");

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
