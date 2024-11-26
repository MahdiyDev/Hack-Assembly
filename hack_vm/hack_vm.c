#include <stdio.h>

#define HACK_VM_PARSER_IMPLEMENTATION
#include "parser.h"
#define UTILS_IMPLEMENTATION
#include "../utils.h"

int main(int argc, char** argv)
{
    char* filename = NULL;
    char* file_path = NULL;

    if (argc < 2) {
        fprintf(stderr, "usage: %s file.vm\n", argv[0]);
        return 1;
    }

    file_path = argv[1];

    if (!(filename = extract_file_name(file_path, "vm"))) {
        fprintf(stderr, "%s is an invalid filename\n", file_path);
        if (filename != NULL) free(filename);
        return 1;
    }
    string_builder* data = sb_init("");
    read_file(file_path, data);

    parser p = parser_init(filename, data);

    string_view parsed_data = parse(p);

    string_builder* file_out = sb_init(filename);
    sb_add_cstr(file_out, ".asm");

    FILE* fp = fopen(sb_to_sv(file_out).data, "w");

    free(filename);
    sb_free(file_out);

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

    parser_destroy(p);
    return 0;
}