#pragma once

#include "../dynamic_array/string.h"

char* dest_str2bin(string_view mnemonic);
char* comp_str2bin(string_view mnemonic);
char* jump_str2bin(string_view mnemonic);

#ifdef HACK_ASM_CONVERT_IMPLEMENTATION
#define UTILS_IMPLEMENTATION
#include "../utils.h"

int find_index(string_view mnemonic, const char* data[], int size)
{
    for (int i = 0; i < size; i++) {
        if (sv_equal_cstr(mnemonic, data[i])) {
            return i;
        }
    }
    return 0;
}

char* dest_str2bin(string_view mnemonic)
{
    const char* data[] = { "", "M", "D", "MD", "A", "AM", "AD", "AMD" };
    int n = find_index(mnemonic, data, arr_count(data));
    return decimal_to_bin(n, 3);
}

char* comp_str2bin(string_view mnemonic)
{
    char* data[][8] = {
        { ""   , "0000000" },
        { "0"  , "0101010" },
        { "1"  , "0111111" },
        { "-1" , "0111010" },
        { "D"  , "0001100" },
        { "A"  , "0110000" },
        { "!D" , "0001101" },
        { "!A" , "0110001" },
        { "-D" , "0001111" },
        { "-A" , "0110011" },
        { "D+1", "0011111" },
        { "A+1", "0110111" },
        { "D-1", "0001110" },
        { "A-1", "0110010" },
        { "D+A", "0000010" },
        { "D-A", "0010011" },
        { "A-D", "0000111" },
        { "D&A", "0000000" },
        { "D|A", "0010101" },
        { "M"  , "1110000" },
        { "!M" , "1110001" },
        { "-M" , "1110011" },
        { "M+1", "1110111" },
        { "M-1", "1110010" },
        { "D+M", "1000010" },
        { "D-M", "1010011" },
        { "M-D", "1000111" },
        { "D&M", "1000000" },
        { "D|M", "1010101" },
    };

    char* bin = "0000000";

    for (int i = 0; i < arr_count(data); i++) {
        if (sv_equal_cstr(mnemonic, data[i][0])) {
            bin = data[i][1];
        }
    }

    return bin;
}

char* jump_str2bin(string_view mnemonic)
{
    const char* data[] = { "", "JGT", "JEQ", "JGE", "JLT", "JNE", "JLE", "JMP" };
    int n = find_index(mnemonic, data, arr_count(data));
    return decimal_to_bin(n, 3);
}
#endif // HACK_ASM_CONVERT_IMPLEMENTATION