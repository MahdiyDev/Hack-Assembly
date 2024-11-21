#include <stdio.h>

#define HACK_ASM_PARSER_IMPLEMENTATION
#include "parser.h"
#define UTILS_IMPLEMENTATION
#include "../utils.h"

int main(int argc, char** argv)
{
    char* filename = NULL;
    char* file_path = NULL;

    if (argc < 2) {
        fprintf(stderr, "usage: %s file.asm\n", argv[0]);
        return 1;
    }

    file_path = argv[1];

    if (!(filename = extract_file_name(file_path, "asm"))) {
        fprintf(stderr, "%s is an invalid filename\n", file_path);
        if (filename != NULL) free(filename);
        return 1;
    }
    string_builder* data = sb_init("");
    read_file(file_path, data);

    parser p = parser_init(filename, data);

    string_view parsed_data = parse(p);

    char* file_out = sb_sprintf(data, "%s.%s", filename, "hack");
    FILE* fp = fopen(file_out, "w");

    fprintf(fp, "%s", parsed_data.data);

    fclose(fp);
    free(file_out);

    parser_destroy(p);
    return 0;
}