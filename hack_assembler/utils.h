#pragma once

char* decimal_to_binary(int n, unsigned int bus_size);
char* bin_to_hex(char* bin, unsigned int bus_size);
char** str_split(char* source, const char* splitter, int* count);

#ifdef UTILS_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* decimal_to_binary(int n, unsigned int bus_size) {
	char* bin;
	int i = 0, b;

	if (!(bin = (char*)malloc(sizeof(char) * bus_size))) {
		fprintf(stderr, "cannot alocate memory!\n");
		return NULL;
	}

	for (b = 14; b >= 0; b--) {
	    int bit = n >> b;

	    if (bit & 1) {
	    	bin[i] = '1';
	    }
	    else {
	     	bin[i] = '0';
	    }
	    i++;
  	}
  	return bin;
}

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
