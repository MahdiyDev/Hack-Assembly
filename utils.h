#pragma once

#ifndef STRING_IMPLEMENTATION
    #define STRING_IMPLEMENTATION
#endif
#include "dynamic_array/string.h"

bool read_file(const char* file_path, string_builder* sb);
char* decimal_to_bin(int n, unsigned int bus_size);
char* bin_to_hex(const char* bin, unsigned int bus_size);
char* extract_file_name(const char* file, const char* extension);

#ifdef UTILS_IMPLEMENTATION
#include <stdio.h>
#include <libgen.h>
char *strndup(const char *str, unsigned long long chars)
{
    char *buffer;
    int n;

    buffer = (char *) malloc(chars +1);
    if (buffer)
    {
        for (n = 0; ((n < chars) && (str[n] != 0)) ; n++) buffer[n] = str[n];
        buffer[n] = 0;
    }

    return buffer;
}

char* extract_file_name(const char* file, const char* extension)
{
    char *bname;
    char *path;
    char* filename;

    string_view file_ext;

    if (!file) {
        return NULL;
    }

    if (!(path = strndup(file, strlen(file)))) {
        return NULL;
    }

    if (!(bname = basename(path))) {
        free(path);
        return NULL;
    }
    
    file_ext = sv_from_cstr(bname);

    string_view file_token = sv_split_c(&file_ext, '.');
    file_ext = sv_split_c(&file_ext, '.');

    if (file_ext.count > 0 && sv_equal_cstr(file_ext, extension)) {
        if (!(filename = (char *)malloc(sizeof(char) * file_token.count + 1))) {   
            return NULL;
        }
        if (!strncpy(filename, file_token.data, file_token.count)) {
            fprintf(stderr, "filename invalid");
            free(filename);
            
            return NULL;
        }
        filename[file_token.count] = '\0';
        free(path);

        return filename;
    }
    fprintf(stderr, "File must have .%s extension\n", extension);
    return NULL;
}

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

char* decimal_to_bin(int n, unsigned int bus_size)
{
	static char bin[64];
	int i = 0, b;

	for (b = bus_size - 1; b >= 0; b--) {
	    int bit = n >> b;

	    if (bit & 1) {
	    	bin[i] = '1';
	    }
	    else {
	     	bin[i] = '0';
	    }
	    i++;
  	}
    bin[bus_size] = '\0';
  	return bin;
}

char* bin_to_hex(const char* bin, unsigned int bus_size)
{
    char* buffer = (char*)malloc(sizeof(char)*bus_size);
    int num = 0;
    do {
        int b = *bin=='1'?1:0;
        num = (num<<1)|b;
        bin++;
    } while (*bin);
    snprintf(buffer, bus_size, "%.4X", num);
    return buffer;
}

#endif // UTILS_IMPLEMENTATION
