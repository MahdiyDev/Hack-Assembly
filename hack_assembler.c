#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <libgen.h>

#define HACK_API_IMPLEMENTATION
#define SYMBOL_TABLE_IMPLEMENTATION
#include "hack_api.h"

instruct_bin_list* parse_instructions(FILE* fp, hack_asm* _asm) {
    char line[INSTR_SIZE];
    char instruct_bin[BUS_SIZE + 1];

    instruct_st* instruct = NULL;
    instruct_bin_list* instruction_list = NULL;
    instruct_bin_list* current_instruction_list = NULL;

    while (fgets(line, sizeof(line), fp)) {
        instruct = parse_instruct(_asm->table, line);

        if (instruct != NULL) {
            char* assembled_instruct = assemble_instruct(instruct, instruct_bin);

            if (assembled_instruct != NULL) {
                if (instruction_list == NULL) {
                    instruction_list = malloc(sizeof(instruct_bin_list));
                    if (instruction_list == NULL) {
                        fprintf(stderr, "cannot alocate memory!\n");
                        return NULL;
                    }
                    current_instruction_list = instruction_list;
                    strncpy(current_instruction_list->instruction, instruct_bin, BUS_SIZE);
                    current_instruction_list->instruction[BUS_SIZE] = '\0';
                    continue;
                }

                current_instruction_list->next = malloc(sizeof(instruct_bin_list));
                if (current_instruction_list->next == NULL) {
                    fprintf(stderr, "cannot alocate memory!\n");
                    return NULL;
                }

                strncpy(current_instruction_list->next->instruction, instruct_bin, BUS_SIZE);
                current_instruction_list->next->instruction[BUS_SIZE] = '\0';

                current_instruction_list = current_instruction_list->next;
                current_instruction_list->next = NULL;
            }
        }
    }

    return instruction_list;
}

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

int main(int argc, char **argv) {
    FILE* fp = NULL;
    char *filename;

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

    _asm.instruct_list = parse_instructions(fp, &_asm);

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