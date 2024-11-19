#pragma once

#ifndef SYMBOL_SIZE
	#define SYMBOL_SIZE 500
#endif // SYMBOL_SIZE
#ifndef PREDEFINED_SYMBOL_COUNT
	#define PREDEFINED_SYMBOL_COUNT 16
#endif // PREDEFINED_SYMBOL_COUNT

typedef struct symbol_table symbol_table;
struct symbol_table {
	char symbol[SYMBOL_SIZE];
	int value;
	symbol_table *next;
};

symbol_table* init_symbol_table();
symbol_table* create_symbol(char* symbol, int value);
symbol_table* insert_to_table(symbol_table* table, char* symbol, int value);
int find_symbol(symbol_table* entry, char* symbol);
void destroy_table(symbol_table* entry);

#ifdef SYMBOL_TABLE_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

symbol_table* create_symbol(char* symbol, int value)
{
    symbol_table* entry = (symbol_table*)malloc(sizeof(symbol_table));
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

void destroy_table(symbol_table* entry)
{
	if (!entry) {
		return;
	}
	symbol_table* next = entry->next;
	free(entry);
	destroy_table(next);
}

symbol_table* init_symbol_table()
{
    symbol_table* table = create_symbol("R0", 0);

    int n;
    char sym[7];
    int registers_count = 16;

    symbol_table* currnet_entry = table;
    for (int i = 1; i < registers_count; i++) {
        if (i < 10 ) {
			n = 3; //'Rx' + '\0'
		} else {
			n = 4; //'Rxx' + '\0'
		}

        snprintf(sym, n, "R%d", i);
		if (!(currnet_entry->next = create_symbol(sym, i)) ) {
			destroy_table(table);
			return NULL;
		}
		currnet_entry = currnet_entry->next;
    }

    return table;
}

symbol_table* insert_to_table(symbol_table* table, char* symbol, int value)
{
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

	if (!(table->next = create_symbol(symbol, value))) {
		goto terminate;
	}
	return table;

terminate:
	destroy_table(table);
	return NULL;	
}

int find_symbol(symbol_table* entry, char* symbol)
{
	while (entry) {
		if (strncmp(entry->symbol, symbol, strlen(symbol)) == 0) {
			return entry->value;
		}
		entry = entry->next;
	}
	return -1;
}

#endif // SYMBOL_TABLE_IMPLEMENTATION
