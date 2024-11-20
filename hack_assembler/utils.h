#pragma once

char* decimal_to_bin(int n, unsigned int bus_size);
char* bin_to_hex(char* bin, unsigned int bus_size);
char** str_split(char* source, const char* splitter, int* count);

#ifdef UTILS_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* bin_to_hex(char* bin, unsigned int bus_size) {
    char* buffer = (char*)malloc(sizeof(char)*bus_size);
    char *a = bin;
    int num = 0;
    do {
        int b = *a=='1'?1:0;
        num = (num<<1)|b;
        a++;
    } while (*a);
    snprintf(buffer, bus_size, "%.4X", num);
    return buffer;
}

char** str_split(char* source, const char* splitter, int* count)
{
    *count = 0;
    char* _source = (char*)malloc(strlen(source) * sizeof(char*));
    strcpy(_source, source);

    char *line = strtok(_source, splitter);
    while (line != NULL) { line = strtok(NULL, splitter); *count += 1; }
    free(_source);

    char** splited_result = (char**)malloc(*count * sizeof(char*));
    line = strtok(source, splitter);

    int i = 0;
    while (line != NULL) {
        char *result = (char*)malloc(strlen(line) + strlen(splitter) + 1); // +1 for the null

        strcpy(result, line);
        strcat(result, splitter);

        splited_result[i] = result;
        i++;

        line = strtok(NULL, splitter);
    }

    return splited_result;
}
#endif // UTILS_IMPLEMENTATION
