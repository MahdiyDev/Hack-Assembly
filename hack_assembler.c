#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <libgen.h>

#define HACK_API_IMPLEMENTATION
#include "hack_api.h"

#define NAME_MAX 256

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

char *extract_file_name(const char *file) {
    char *file_token;
    char *ext_token;
    char *filename;
    char *bname;
    char file_cpy[NAME_MAX];
    char *path;

    if (!file) {
        return NULL;
    }

    if (strlen(file) > NAME_MAX) {
        fprintf(stderr, "File name exceeds %d characters\n", NAME_MAX);
        return NULL;
    }

    if (! (path = strndup(file, strlen(file))) ) {
        return NULL;
    }

    if (! (bname = basename(path)) ) {
        free(path);
        return NULL;
    }
    
    strncpy(file_cpy, bname, strlen(bname));
    file_cpy[strlen(file)] = '\0';

    free(path);
    
    file_token = strtok(file_cpy, ".");
    ext_token  = strtok(NULL, ".");

    if (file_token && ext_token && strncmp(ext_token, "asm", 3) == 0) {
        if (!(filename = (char *)malloc(sizeof(char) * strlen(file_token) + 1))) {
            
            return NULL;
        }
        if (!strncpy(filename, file_token, strlen(file_token))) {
            fprintf(stderr, "filename invalid");
            free(filename);
            
            return NULL;
        }
        filename[strlen(file_token)] = '\0';
        
        return filename;
    }
    fprintf(stderr, "File must have .asm extension\n");
    return NULL;
}

int write_binary_instructions(char *filename, instruct_bin_list *instructions, bool is_hex) {
    instruct_bin_list *instruction = instructions;
    FILE *fp;
    int name_size = strlen(filename) + 5 + 1;
    char *fileout_name;

    if (!(fileout_name = malloc(sizeof(char) * name_size)) ) {
		fprintf(stderr, "cannot alocate memory!\n");
        return 0;
    }

    strncpy(fileout_name, filename, strlen(filename));
    fileout_name[strlen(filename)] = '\0';
    strncat(fileout_name, ".hack", 6);
    fileout_name[strlen(filename) + 5] = '\0';
    

    if (!(fp = fopen(fileout_name, "w")) ) {
        free(fileout_name);
    }
    while (instruction) {
        if (is_hex) {
            fprintf(fp, "%s ", bin_to_hex(instruction->instruction, BUS_SIZE));
        } else {
            fprintf(fp, "%s\n", instruction->instruction);
        }
        instruction = instruction->next;
    }
    free(fileout_name);
    fclose(fp);
    return 0;
}

char* readfile(FILE* fp)
{
    long file_size;
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    char* text_data = (char*)malloc(file_size);
    int ch;
    int i = 0;
    while ((ch = fgetc(fp)) != EOF) {
        text_data[i] = ch;
        i++;
    }
    return text_data;
}

int main(int argc, char **argv) {
    FILE* fp = NULL;
    char *filename;
    long file_size;

    if (argc < 2) {
        fprintf(stderr, "usage: %s file.asm\n", argv[0]);
        return 1;
    }

    if (!(filename = extract_file_name(argv[1]))) {
        fprintf(stderr, "%s is an invalid filename\n", argv[1]);
        goto cleanup;
    }
    
    if (!(fp = fopen(argv[1], "r"))) {
        fprintf(stderr, "Failed to open file: %s\n", argv[1]);
        goto cleanup;
    }

    hack_asm _asm = init_hack_asm();

    char* source = readfile(fp);

    parse_instructions(source, &_asm);

    bool is_hex = false;
    if (argc > 2) {
        is_hex = strcmp(argv[2], "--hex") == 0;
    }

    return write_binary_instructions(filename, _asm.instruct_list, is_hex);

cleanup:
    if (fp) {
        fclose(fp);
    }

    if (filename) {
        free(filename);
    }
    
    destroy_hack_asm(&_asm);
    
    return 0;
}