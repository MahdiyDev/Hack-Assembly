#pragma once

#ifdef STRING_IMPLEMENTATION
    #define STRING_IMPLEMENTATION
#endif // STRING_IMPLEMENTATION
#include "../dynamic_array/string.h"

typedef struct {
    string_view symbol;
    size_t value;
} symbol_table_item;

typedef struct {
    symbol_table_item* items;
    size_t capacity;
    size_t count;

    int next_address;
} symbol_table;

symbol_table* st_init();
void st_destroy(symbol_table* st);

symbol_table_item* st_find(symbol_table* st, string_view symbol);
symbol_table_item* st_put(symbol_table* st, string_view symbol, int value);
symbol_table_item* st_get(symbol_table* st, string_view symbol);

void st_put_label(symbol_table* st, string_view symbol, int address);
int st_put_variable(symbol_table* st, string_view symbol);

#ifdef SYMBOL_TABLE_IMPLEMENTATION
#include <stddef.h>

symbol_table* st_init()
{
    symbol_table* st;
    da_init(st);

    // default variables
    st_put(st, sv_from_cstr("SP"), 0);
    st_put(st, sv_from_cstr("LCL"), 1);
    st_put(st, sv_from_cstr("ARG"), 2);
    st_put(st, sv_from_cstr("THIS"), 3);
    st_put(st, sv_from_cstr("THAT"), 4);
    st_put(st, sv_from_cstr("SCREEN"), 16384);
    st_put(st, sv_from_cstr("KBD"), 24576);

    for (int i = 0; i < 16; i++) {
        st_put(st, sv_from_cstr(sb_sprintf("R%d", i)), i);
    }

    st->next_address = 16;

    return st;
}

void st_destroy(symbol_table* st)
{
    da_free(st);
}

symbol_table_item* st_put(symbol_table* st, string_view symbol, int value)
{
    symbol_table_item* found_table = st_find(st, symbol);
    if (found_table == NULL) {
        symbol_table_item new_item = { .symbol = symbol };
        found_table = &new_item;
        found_table->value = value;
        da_append(st, *found_table);
    }
    found_table->value = value;
    return found_table;
}

void st_put_label(symbol_table* st, string_view symbol, int address)
{
    st_put(st, symbol, address);
}

int st_put_variable(symbol_table* st, string_view symbol)
{
    st_put(st, symbol, st->next_address);
    st->next_address += 1;
    return st->next_address - 1;
}

symbol_table_item* st_find(symbol_table* st, string_view symbol)
{
    for (int i = 0 ; i < st->count; i++) {
        if (sv_equal(st->items[i].symbol, symbol)) return &st->items[i];
    }
    return NULL;
}
#endif // SYMBOL_TABLE_IMPLEMENTATION
