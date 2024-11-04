#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <libgen.h>

#define BUS_SIZE    16
#define SYMBOL_SIZE 500
#define INSTR_SIZE  150
#define PREDEFINED_SYMBOL_COUNT 16

#define JUMP_SIZE 3
#define DEST_SIZE 3
#define COMP_SIZE 6

//INSTRUCTION HEADER
#define C_HEADER   "111"
#define A_HEADER   "0"

#define C_HEADER_SIZE 3
#define A_HEADER_SIZE 1

//JUMPS
#define NULL_J     "000"
#define JGT        "001"
#define JEQ        "010"
#define JGE        "011"
#define JLT        "100"
#define JNE        "101"
#define JLE        "110"
#define JMP        "111"

//DESTINATION
#define NULL_DEST  "000"
#define M_DEST     "001"
#define D_DEST     "010"
#define MD_DEST    "011"
#define A_DEST     "100"
#define AM_DEST    "101"
#define AD_DEST    "110"
#define AMD_DEST   "111"

/*** C-INSTRUCTIONS ***/
//a = 0
#define ZERO       "0101010"
#define ONE        "0111111"
#define NEG_ONE    "0111010"
#define D          "0001100"
#define A          "0110000"
#define NOT_D      "0001101"
#define NOT_A      "0110001"
#define MINUS_D    "0001111"
#define MINUS_A    "0110011"
#define D_PLUS_1   "0011111"
#define A_PLUS_1   "0110111"
#define D_MINUS_1  "0001110"
#define A_MINUS_1  "0110010"
#define D_PLUS_A   "0000010"
#define D_MINUS_A  "0010011"
#define A_MINUS_D  "0000111"
#define D_AND_A    "0000000"
#define D_OR_A     "0010101"

//a = 1
#define M          "1110000"
#define NOT_M      "1110001"
#define MINUS_M    "1110011"
#define M_PLUS_1   "1110111"
#define M_MINUS_1  "1110010"
#define D_PLUS_M   "1000010"
#define D_MINUS_M  "1010011"
#define M_MINUS_D  "1000111"
#define D_AND_M    "1000000"
#define D_OR_M     "1010101"

typedef enum {
    A_INSTRUCTION = 0,
    C_INSTRUCTION,
    INVALID_INSTRUCTION,
} instruction_type;

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
	char *header_bin;
	union instruct_blob blob;
} instruct_st;

typedef struct symbol_entry symbol_entry;
struct symbol_entry {
	char symbol[SYMBOL_SIZE];
	int value;
	symbol_entry *next;
};

typedef struct instruct_bin_entry instruct_bin_entry;
struct instruct_bin_entry {
	char instruction[BUS_SIZE + 1];
	struct instruct_bin_entry *next;
};

char* decimal_to_binary(int n) {
	char *bin;
	int i = 0, b;

	if (!(bin = malloc(sizeof(char) * BUS_SIZE))) {
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

char *jump_str2bin(char *jmp_str) {
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

char *dest_str2bin(char *dest_str) {
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

char *comp_str2bin(char *comp_str) {
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


int find_symbol(struct symbol_entry *entry, char *symbol) {
	while (entry) {
		if (strncmp(entry->symbol, symbol, strlen(symbol)) == 0) {
			return entry->value;
		}
		entry = entry->next;
	}
	return -1;
}


symbol_entry* create_sym_entry(char* symbol, int value) {
    symbol_entry* entry = malloc(sizeof(symbol_entry));
    size_t symbol_len = strlen(symbol);
    
    if (entry == NULL) {
		fprintf(stderr, "cannot alocate memory!\n");
        return NULL;
    }
    
    if (!symbol || symbol_len > SYMBOL_SIZE) {
		fprintf(stderr, "symbol is empty or exceeds over %d character limit\n", SYMBOL_SIZE);
		return NULL;
	}

    strncpy(entry->symbol, symbol, symbol_len);
	entry->symbol[symbol_len] = '\0';
	entry->value = value;
	entry->next = NULL;
    
    return entry;
}

void destroy_table(symbol_entry* entry) {
	if (!entry) {
		return;
	}
	struct symbol_entry *next = entry->next;
	free(entry);
	destroy_table(next);
}

symbol_entry* init_symbol_entry() {
    symbol_entry* table = create_sym_entry("R0", 0);

    int n;
    char sym[7];
    int registers_count = 16;

    symbol_entry *currnet_entry = table;
    for (int i = 1; i < registers_count; i++) {
        if (i < 10 ) {
			n = 3; //'Rx' + '\0'
		} else {
			n = 4; //'Rxx' + '\0'
		}

        snprintf(sym, n, "R%d", i);
		if (!(currnet_entry->next = create_sym_entry(sym, i)) ) {
			destroy_table(table);
			return NULL;
		}
		currnet_entry = currnet_entry->next;
    }

    return table;
}

symbol_entry *insert_to_table(symbol_entry *table, char * symbol, int value) {
	int i = 0;
    static int symbol_count = 0;

	if (!table) {
		fprintf(stderr, "table cannot be NULL\n");
		return NULL;
	}

	while (table->next) {
		table = table->next;
		i++;
	}

	if (i < PREDEFINED_SYMBOL_COUNT - 1) {
		fprintf(stderr, "Table is missing pre-defined symbols\n");
		goto terminate;
	}

	//if address needs to be dynamically allocated
	if (value < 0) {
		value = 16 + symbol_count;
		symbol_count++;
	}

	if (! (table->next = create_sym_entry(symbol, value)) ) {
		goto terminate;
	}
	return table;

terminate:
	destroy_table(table);
	return NULL;	
}

bool is_label_check(char* instruction_str) {
    for (int i = 0; i < strlen(instruction_str); i++) {
        if (instruction_str[i] == '\r' || instruction_str[i] == '\n') {
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

char* parse_address(symbol_entry* table, char* instruction_str, bool is_label) {
    int value = -1;
    
    int instruction_len = strlen(instruction_str);
    if (instruction_str[0] == '(') {
        int del_index = instruction_len - 2; // last character index
        memmove(&instruction_str[del_index], &instruction_str[del_index + 1], instruction_len - del_index);
    }
    instruction_str += 1; // move pointer to second position of token

    if (isdigit(instruction_str[0])) {
        for (int i = 0; i < strlen(instruction_str); i++) {
            if (instruction_str[i] == '\r' || instruction_str[i] == '\n') {
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
        return decimal_to_binary(value);
    }
    return NULL;
}

instruct_st* parse_instruction_a(symbol_entry* table, char* instruction_str, instruct_st* instruct) {
    bool is_label = is_label_check(instruction_str);
    char* addr = parse_address(table, instruction_str, is_label);
    line_count++;

    if (addr == NULL) {
        return NULL;
    }

    instruct->type = A_INSTRUCTION;
    instruct->header_bin = A_HEADER;
    instruct->blob.blob_a.is_label = is_label;
	strncpy(instruct->blob.blob_a.address, addr, 15);
    instruct->blob.blob_a.address[15] = '\0';

    return instruct;
}

char *tokenize_instruction(char *instr_str, int dest_pos, int comp_pos, int jump_pos) {
    if (!instr_str || !(*instr_str)) {
        fprintf(stderr, "instruction is NULL\n");
        return NULL;
    }

    instr_str[dest_pos] = '\0';
    instr_str[comp_pos] = '\0';
    instr_str[jump_pos] = '\0';

    return instr_str;
}

instruct_st* parse_instruction_c(char* source, instruct_st* instruct) {
    int i = 0;
    int state = 0;
    int dest_end = 0, comp_end = 0, jump_end = 0;
    int dest_start = -1, comp_start = -1, jump_start = -1, tmp_start = -1;

    char instr_str[INSTR_SIZE];
    char *jump_bin;
    char *dest_bin;
    char *comp_bin;

    strncpy(instr_str, source, strlen(source) % INSTR_SIZE);
    instr_str[strlen(source)] = '\0';

    for (i = 0; i < strlen(instr_str); i++) {
        switch (state) {
            case 0: {
                if (tmp_start < 0 && isalnum(instr_str[i])) {
                    tmp_start = i;
                    break;
                }
                else if (instr_str[i] == '=') {
                    dest_end   = i;
                    dest_start = tmp_start;
                    comp_start = i + 1;
                    state++; //search for computation instruction
                    break;
                }
                else if (instr_str[i] == '/' || instr_str[i] == ' ') {
                    return NULL;
                }
            }
            case 1: {
                if (instr_str[i] == ';') {
                    comp_end = i;
                    state = 2; //search for JUMP instruction
                    jump_start = i + 1;
                    if (dest_start < 0) {
                        comp_start = tmp_start;
                    }
                    break;
                }
                else if (instr_str[i] == '\r' || instr_str[i] == '\n' ||
                         instr_str[i] == '/'  || instr_str[i] == ' ') 
                {
                    comp_end = i;
                    i = strlen(instr_str);
                    break;
                }
            }
            case 2: {
                if (instr_str[i] == ' ') {
                    int spaces = 0;
                    for (int k = 0; instr_str[k] != '\0'; k++) {
                        if (instr_str[k] == ' ') {
                            spaces++;
                        }
                    }
                    jump_start += spaces;
                }
                if (instr_str[i] == '\r' || instr_str[i] == '\n' ||
                    instr_str[i] == ' ') 
                {
                    jump_end = i;
                    i = strlen(instr_str);
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
        dest_end = strlen(instr_str);
        dest_start = dest_end;
    }

    if (jump_start < 1) {
        jump_end = strlen(instr_str);
        jump_start = jump_end;
    }

    tokenize_instruction(source, dest_end, comp_end, jump_end);

    if (!(dest_bin = dest_str2bin(source+dest_start)) ) {
        fprintf(stderr, "destination token is invalid\n");
        return NULL;
    }

    if (!(comp_bin = comp_str2bin(source+comp_start)) ) {
        fprintf(stderr, "computation token is invalid\n");
        return NULL;
    }

    if (!(jump_bin = jump_str2bin(strtok(source+jump_start, "\n"))) ) {
        fprintf(stderr, "jump token is invalid\n");
        return NULL;
    }

    instruct->type = C_INSTRUCTION;
    instruct->header_bin = C_HEADER;
    strncpy(instruct->blob.blob_c.dest_bin, dest_bin, DEST_SIZE);
    instruct->blob.blob_c.dest_bin[DEST_SIZE] = '\0';
    strncpy(instruct->blob.blob_c.comp_bin, comp_bin, COMP_SIZE + 1);
    instruct->blob.blob_c.comp_bin[COMP_SIZE + 1] = '\0';
    strncpy(instruct->blob.blob_c.jump_bin, jump_bin, JUMP_SIZE);
    instruct->blob.blob_c.jump_bin[COMP_SIZE - 3] = '\0';

    return instruct;
}

instruct_st* parse_instruction(symbol_entry* table, char* instruction_str) {
    instruct_st* instruct = malloc(sizeof(instruct_st));
    if (instruct == NULL) {
		fprintf(stderr, "cannot alocate memory!\n");
		return NULL;
    }
    int instruction_len = strlen(instruction_str);
    for (int i = 0; i < instruction_len; i++) {
        if (instruction_str[i] == ' ') {
            continue;
        }
        else if (instruction_str[i] == '@' || instruction_str[i] == '(') {
            return parse_instruction_a(table, instruction_str, instruct);
        }
        else if (isalnum(instruction_str[i])) {
            // line_count++;
            return parse_instruction_c(instruction_str, instruct);
        }
        else if (instruction_str[i] == '/') {
            return NULL;
        }
    }

    return NULL;
}

char *assemble_instruct(instruct_st *instruct, char *instruct_bin) {
    if (!instruct || !instruct_bin) {
        return NULL;
    }
    if (instruct->type == A_INSTRUCTION && !instruct->blob.blob_a.is_label)
    {
        snprintf(instruct_bin, BUS_SIZE + 1,
            "%s%s", instruct->header_bin, instruct->blob.blob_a.address
        );
    }
    else if (instruct->type == C_INSTRUCTION) {
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

void destroy_instructions(instruct_bin_entry *instr) {
    if (!instr) {
        return;
    }
    struct instruct_bin_entry *next = instr->next;
    free(instr);
    destroy_instructions(next);
}

instruct_bin_entry* parse_instructions(FILE* fp, symbol_entry* table) {
    char line[INSTR_SIZE];
    char instruct_bin[BUS_SIZE + 1];

    instruct_st* instruct;
    instruct_bin_entry* instruction_list = NULL;
    instruct_bin_entry* current_instruction_list = NULL;

    while (fgets(line, sizeof(line), fp)) {
        instruct = parse_instruction(table, line);

        if (instruct != NULL) {
            char* assembled_instruct = assemble_instruct(instruct, instruct_bin);

            if (assembled_instruct != NULL) {
                if (instruction_list == NULL) {
                    instruction_list = malloc(sizeof(instruct_bin_entry));
                    if (instruction_list == NULL) {
                        fprintf(stderr, "cannot alocate memory!\n");
                        return NULL;
                    }
                    current_instruction_list = instruction_list;
                    strncpy(current_instruction_list->instruction, instruct_bin, BUS_SIZE);
                    current_instruction_list->instruction[BUS_SIZE] = '\0';
                    continue;
                }

                current_instruction_list->next = malloc(sizeof(instruct_bin_entry));
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

char* bin_to_hex(char *bin) {
    char* buffer = malloc(sizeof(char)*BUS_SIZE);
    char *a = bin;
    int num = 0;
    do {
        int b = *a=='1'?1:0;
        num = (num<<1)|b;
        a++;
    } while (*a);
    snprintf(buffer, BUS_SIZE, "%.4X", num);
    return buffer;
}

int write_binary_instructions(char *filename, struct instruct_bin_entry *instructions, bool is_hex) {
    instruct_bin_entry *instruction = instructions;
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
            fprintf(fp, "%s ", bin_to_hex(instruction->instruction));
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
    symbol_entry* table = NULL;

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

    table = init_symbol_entry();

    instruct_bin_entry* instructions = parse_instructions(fp, table);

    bool is_hex = false;
    if (argc > 2) {
        is_hex = strcmp(argv[2], "--hex") == 0;
    }

    return write_binary_instructions(filename, instructions, is_hex);

    cleanup:
        if (fp) {
            fclose(fp);
        }

        if (filename) {
            free(filename);
        }

        if (table) {
            destroy_table(table);
        }

        if (instructions) {
            destroy_instructions(instructions);
        }
        return 0;
}