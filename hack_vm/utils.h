#pragma once
#include <stdio.h>

#ifndef STRING_IMPLEMENTATION
    #define STRING_IMPLEMENTATION
#endif
#include "../dynamic_array/string.h"

bool read_file(const char* file_path, string_builder* sb);

#ifdef UTILS_IMPLEMENTATION
bool read_file(const char* file_path, string_builder* sb)
{
    bool result = true;

    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        if (!result) fprintf(stderr, "Could not read file %s: %s", file_path, strerror(errno));
        if (file) fclose(file);
        return result;
    }

    int ch;
    while ((ch = fgetc(file)) != EOF) {
        sb_add_c(sb, ch);
    }

    if (ferror(file)) {
        if (!result) fprintf(stderr, "Could not read file %s: %s", file_path, strerror(errno));
        if (file) fclose(file);
        return result;
    }

    fclose(file);
    return result;
}
#endif // UTILS_IMPLEMENTATION
