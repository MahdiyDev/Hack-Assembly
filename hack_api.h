#pragma once

#include <stdbool.h>

#define SYMBOL_TABLE_IMPLEMENTATION
#include "symbol_table.h"
#include "code.h"

#define BUS_SIZE    16
#define INSTR_SIZE  150

struct instruct_a_blob {
	char address[BUS_SIZE];
	int  is_label;
};

struct instruct_c_blob {
	char comp_bin[COMP_SIZE + 2]; //+1 for the a-bit to indicate whether it
                                  // tries to use the M register
	char dest_bin[DEST_SIZE + 1];
	char jump_bin[JUMP_SIZE + 1];
};

union instruct_blob {
	struct instruct_a_blob blob_a;
	struct instruct_c_blob blob_c;
};

typedef struct {
	instruction_type type;
	char* header_bin;
	union instruct_blob blob;
} instruct_st;

typedef struct instruct_bin_list instruct_bin_list;
struct instruct_bin_list {
	char instruction[BUS_SIZE + 1];
	instruct_bin_list* next;
};

typedef struct {
    symbol_table* table;
    instruct_bin_list* instruct_list;
} hack_asm;

hack_asm init_hack_asm();
void destroy_hack_asm(hack_asm* _asm);
void parse_instructions(char *source, hack_asm* _asm);

instruct_st* parse_instruct(symbol_table* table, char* source);
instruct_st* create_instruct_a(symbol_table* table, char* instruction_str, instruct_st* instruct);
instruct_st* create_instruct_c(symbol_table* table, char* instruction_str, instruct_st* instruct);
char* assemble_instruct(instruct_st* instruct, char* instruct_bin);

#ifdef HACK_API_IMPLEMENTATION
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define UTILS_IMPLEMENTATION
#include "utils.h"

hack_asm init_hack_asm()
{
    hack_asm _asm;
    _asm.table = init_symbol_table();

    return _asm;
}

void destroy_instructions(instruct_bin_list *instr)
{
    if (!instr) {
        return;
    }
    instruct_bin_list *next = instr->next;
    free(instr);
    destroy_instructions(next);
}

void destroy_hack_asm(hack_asm* _asm)
{
    destroy_table(_asm->table);
    destroy_instructions(_asm->instruct_list);
}

void parse_instructions(char *source, hack_asm* _asm) {
    char instruct_bin[BUS_SIZE + 1];

    instruct_st* instruct = NULL;
    instruct_bin_list* current_instruction_list = NULL;

    int count = 0;
    char** lines = str_split(source, "\n", &count);

    for (int i = 0; i < count; i++) {
        instruct = parse_instruct(_asm->table, lines[i]);

        if (instruct == NULL) continue;
        char* assembled_instruct = assemble_instruct(instruct, instruct_bin);

        if (assembled_instruct == NULL) continue;

        if (_asm->instruct_list == NULL) {
            _asm->instruct_list = malloc(sizeof(instruct_bin_list));
            if (_asm->instruct_list == NULL) {
                fprintf(stderr, "cannot alocate memory!\n");
                return;
            }
            current_instruction_list = _asm->instruct_list;
            current_instruction_list->next = NULL;
            strncpy(current_instruction_list->instruction, instruct_bin, BUS_SIZE);
            current_instruction_list->instruction[BUS_SIZE] = '\0';
            continue;
        }

        current_instruction_list->next = malloc(sizeof(instruct_bin_list));
        if (current_instruction_list->next == NULL) {
            fprintf(stderr, "cannot alocate memory!\n");
            return;
        }

        strncpy(current_instruction_list->next->instruction, instruct_bin, BUS_SIZE);
        current_instruction_list->next->instruction[BUS_SIZE] = '\0';

        current_instruction_list = current_instruction_list->next;
        current_instruction_list->next = NULL;
    }
}

instruct_st* parse_instruct(symbol_table* table, char* instruction_str)
{
    instruct_st* instruction = (instruct_st*)malloc(sizeof(instruct_st));
    for (int i = 0; i < strlen(instruction_str); i++) {
        if (instruction_str[i] == ' ' || instruction_str[i] == '\t') {
            continue;
        }
        else if (instruction_str[i] == '@' || instruction_str[i] == '(') {
            return create_instruct_a(table, instruction_str, instruction);
        }
        else if (isalnum(instruction_str[i])) {
            return create_instruct_c(table, instruction_str, instruction);
        }
        else if (instruction_str[i] == '/') {
            return NULL;
        }
    }

    return NULL;
}

char* assemble_instruct(instruct_st* instruct, char* instruct_bin) {
    if (!instruct || !instruct_bin) {
        return NULL;
    }
    if (instruct->type == A_INSTR && !instruct->blob.blob_a.is_label)
    {
        snprintf(instruct_bin, BUS_SIZE + 1,
            "%s%s", instruct->header_bin, instruct->blob.blob_a.address
        );
    }
    else if (instruct->type == C_INSTR) {
        snprintf(instruct_bin,
            C_HEADER_SIZE + DEST_SIZE + COMP_SIZE + 1 + JUMP_SIZE + 1,
            "%s%s%s%s",
            instruct->header_bin, instruct->blob.blob_c.comp_bin,
            instruct->blob.blob_c.dest_bin, instruct->blob.blob_c.jump_bin
        );
    }
    else {
        return NULL;
    }
    return instruct_bin;
}

bool is_new_line(const char source)
{
    return source == '\r' || source == '\n';
}

bool is_label_check(char* instruction_str)
{
    for (int i = 0; i < strlen(instruction_str); i++) {
        if (is_new_line(instruction_str[i])) {
            break;
        }
        else if (instruction_str[i] == ')') {
            return true;
        }
        else if (instruction_str[i] == '(' || instruction_str[i] == ' ') {
            continue;
        }
    }
    return false;
}

int line_count = 0;

char* parse_address(symbol_table* table, char* instruction_str, bool is_label)
{
    int value = -1;
    
    int instruction_len = strlen(instruction_str);
    if (instruction_str[0] == '(') {
        int del_index = instruction_len - 2; // last character index
        memmove(&instruction_str[del_index], &instruction_str[del_index + 1], instruction_len - del_index);
    }
    instruction_str += 1; // move pointer to second position of token

    if (isdigit(instruction_str[0])) {
        for (int i = 0; i < strlen(instruction_str); i++) {
            if (is_new_line(instruction_str[i])) {
                break;
            }
            if (!isdigit(instruction_str[i])) {
                fprintf(stderr, "In line: %s\nNot a valid numeric address\n", instruction_str);
                return NULL;
            }
        }
        value = strtol(instruction_str, NULL, 10);
    }
    else if (isalpha(instruction_str[0])) {
        value = find_symbol(table, instruction_str);
		if (value < 0) {
            if (is_label) {
                value = line_count;
            }
            line_count++;
			if (!insert_to_table(table, instruction_str, value)) {
				return NULL;
			}
			value = find_symbol(table, instruction_str);
		}
    }

    if (value >= 0) {
        return decimal_to_binary(value, BUS_SIZE);
    }
    return NULL;
}

instruct_st* create_instruct_a(symbol_table* table, char* instruction_str, instruct_st* instruct)
{
    bool is_label = is_label_check(instruction_str);
    char* addr = parse_address(table, instruction_str, is_label);
    line_count++;

    if (addr == NULL) {
        return NULL;
    }

    instruct->type = A_INSTR;
    instruct->header_bin = A_HEADER;
    instruct->blob.blob_a.is_label = is_label;
	strncpy(instruct->blob.blob_a.address, addr, 15);
    instruct->blob.blob_a.address[15] = '\0';

    return instruct;
}

char* tokenize_instruction(char* instr_str, int dest_pos, int comp_pos, int jump_pos)
{
    if (!instr_str || !(*instr_str)) {
        fprintf(stderr, "instruction is NULL\n");
        return NULL;
    }

    instr_str[dest_pos] = '\0';
    instr_str[comp_pos] = '\0';
    instr_str[jump_pos] = '\0';

    return instr_str;
}

char *jump_str2bin(char *jmp_str)
{
	if (!jmp_str || strlen(jmp_str) == 0) {
		return NULL_J;
	}
	else if (strcmp(jmp_str, "JGT") == 0) {
		return JGT;
	}
	else if (strcmp(jmp_str, "JEQ") == 0) {
		return JEQ;
	}
	else if (strcmp(jmp_str, "JGE") == 0) { 
		return JGE;
	}
	else if (strcmp(jmp_str, "JLT") == 0) {
		return JLT;
	}
	else if (strcmp(jmp_str, "JNE") == 0) {
		return JNE;
	}
	else if (strcmp(jmp_str, "JLE") == 0) {
		return JLE;
	}
	else if (strcmp(jmp_str, "JMP") == 0) {
		return JMP;
	}
	return NULL;
}

char *dest_str2bin(char *dest_str)
{
	if (!dest_str || strlen(dest_str) == 0) {
		return NULL_DEST;
	}
	else if (strcmp(dest_str, "M") == 0) {
		return M_DEST;
	}
	else if (strcmp(dest_str, "D") == 0) {
		return D_DEST;
	}
	else if (strcmp(dest_str, "MD") == 0) {
		return MD_DEST;
	}
	else if (strcmp(dest_str, "A") == 0) {
		return A_DEST;
	}
	else if (strcmp(dest_str, "AM") == 0) {
		return AM_DEST;
	}
	else if (strcmp(dest_str, "AD") == 0) {
		return AD_DEST;
	}
	else if (strcmp(dest_str, "AMD") == 0) {
		return AMD_DEST;
	}
	return NULL;
}

char *comp_str2bin(char *comp_str)
{
	if (!comp_str) {
		return NULL;
	}
	else if (strcmp(comp_str, "0") == 0) {
		return ZERO;
	}
	else if (strcmp(comp_str, "1") == 0) {
		return ONE;
	}
	else if (strcmp(comp_str, "-1") == 0) {
		return NEG_ONE;
	}
	else if (strcmp(comp_str, "D") == 0) {
		return D;
	}
	else if (strcmp(comp_str, "A") == 0) {
		return A;
	}
	else if (strcmp(comp_str, "!D") == 0) {
		return NOT_D;
	}
	else if (strcmp(comp_str, "!A") == 0) {
		return NOT_A;
	}
	else if (strcmp(comp_str, "-D") == 0) {
		return MINUS_D;
	}
	else if (strcmp(comp_str, "-A") == 0) {
		return MINUS_A;
	}
	else if (strcmp(comp_str, "D+1") == 0) {
		return D_PLUS_1;
	}
	else if (strcmp(comp_str, "A+1") == 0) {
		return A_PLUS_1;
	}
	else if (strcmp(comp_str, "D-1") == 0) {
		return D_MINUS_1;
	}
	else if (strcmp(comp_str, "A-1") == 0) {
		return A_MINUS_1;
	}
	else if (strcmp(comp_str, "D+A") == 0) {
		return D_PLUS_A;
	}
	else if (strcmp(comp_str, "D-A") == 0) {
		return D_MINUS_A;
	}
	else if (strcmp(comp_str, "A-D") == 0) {
		return A_MINUS_D;
	}
	else if (strcmp(comp_str, "D&A") == 0) {
		return D_AND_A;
	}
	else if (strcmp(comp_str, "D|A") == 0) {
		return D_OR_A;
	}

	//a = 1
	else if (strcmp(comp_str, "M") == 0) {
		return M;
	}
	else if (strcmp(comp_str, "!M") == 0) {
		return NOT_M;
	}
	else if (strcmp(comp_str, "-M") == 0) {
		return MINUS_M;
	}
	else if (strcmp(comp_str, "M+1") == 0) {
		return M_PLUS_1;
	}
	else if (strcmp(comp_str, "M-1") == 0) {
		return M_MINUS_1;
	}
	else if (strcmp(comp_str, "D+M") == 0) {
		return D_PLUS_M;
	}
	else if (strcmp(comp_str, "D-M") == 0) {
		return D_MINUS_M;
	}
	else if (strcmp(comp_str, "M-D") == 0) {
		return M_MINUS_D;
	}
	else if (strcmp(comp_str, "D&M") == 0) {
		return D_AND_M;
	}
	else if (strcmp(comp_str, "D|M") == 0) {
		return D_OR_M;
	}
	return NULL;
}

instruct_st* create_instruct_c(symbol_table* table, char* instr_str, instruct_st* instruct)
{
    int i = 0;
    int state = 0;
    int dest_end = 0, comp_end = 0, jump_end = 0;
    int dest_start = -1, comp_start = -1, jump_start = -1, tmp_start = -1;

    char new_instr_str[INSTR_SIZE];
    char *jump_bin;
    char *dest_bin;
    char *comp_bin;

    strncpy(new_instr_str, instr_str, strlen(instr_str) % INSTR_SIZE);
    new_instr_str[strlen(instr_str)] = '\0';

    for (i = 0; i < strlen(new_instr_str); i++) {
        switch (state) {
            case 0: {
                if (tmp_start < 0 && isalnum(new_instr_str[i])) {
                    tmp_start = i;
                    break;
                }
                else if (new_instr_str[i] == '=') {
                    dest_end   = i;
                    dest_start = tmp_start;
                    comp_start = i + 1;
                    state++; //search for computation instruction
                    break;
                }
                else if (new_instr_str[i] == '/' || new_instr_str[i] == ' ') {
                    return NULL;
                }
            }
            case 1: {
                if (new_instr_str[i] == ';') {
                    comp_end = i;
                    state = 2; //search for JUMP instruction
                    jump_start = i + 1;
                    if (dest_start < 0) {
                        comp_start = tmp_start;
                    }
                    break;
                }
                else if (is_new_line(new_instr_str[i]) || new_instr_str[i] == '/'  || new_instr_str[i] == ' ') 
                {
                    comp_end = i;
                    i = strlen(new_instr_str);
                    break;
                }
            }
            case 2: {
                if (new_instr_str[i] == ' ') {
                    int spaces = 0;
                    for (int k = 0; new_instr_str[k] != '\0'; k++) {
                        if (new_instr_str[k] == ' ') {
                            spaces++;
                        }
                    }
                    jump_start += spaces;
                }
                if (is_new_line(new_instr_str[i]) || new_instr_str[i] == ' ') 
                {
                    jump_end = i;
                    i = strlen(new_instr_str);
                }
                break;
            }
            default:
                break;
        }
    }

    if (comp_end == 0) {
        return NULL;
    }

    if (dest_end < 1) {
        dest_end = strlen(new_instr_str);
        dest_start = dest_end;
    }

    if (jump_start < 1) {
        jump_end = strlen(new_instr_str);
        jump_start = jump_end;
    }

    tokenize_instruction(instr_str, dest_end, comp_end, jump_end);

    if (!(dest_bin = dest_str2bin(instr_str+dest_start)) ) {
        fprintf(stderr, "destination token is invalid\n");
        return NULL;
    }

    if (!(comp_bin = comp_str2bin(instr_str+comp_start)) ) {
        fprintf(stderr, "computation token is invalid\n");
        return NULL;
    }

    if (!(jump_bin = jump_str2bin(strtok(instr_str+jump_start, "\n"))) ) {
        fprintf(stderr, "jump token is invalid\n");
        return NULL;
    }

    instruct->type = C_INSTR;
    instruct->header_bin = C_HEADER;
    strncpy(instruct->blob.blob_c.dest_bin, dest_bin, DEST_SIZE);
    instruct->blob.blob_c.dest_bin[DEST_SIZE] = '\0';
    strncpy(instruct->blob.blob_c.comp_bin, comp_bin, COMP_SIZE + 1);
    instruct->blob.blob_c.comp_bin[COMP_SIZE + 1] = '\0';
    strncpy(instruct->blob.blob_c.jump_bin, jump_bin, JUMP_SIZE);
    instruct->blob.blob_c.jump_bin[JUMP_SIZE] = '\0';

    return instruct;
}
#endif // HACK_API_IMPLEMENTATION
