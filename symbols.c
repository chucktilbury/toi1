/*
    Store symbols and their attributes
*/

#include <stdio.h>
#include <stdlib.h>

#include "sym_attrs.h"
#include "errors.h"
#include "logging.h"
#include "symbols.h"
#include "hash_table.h"

typedef void **sattr_table_t;
typedef ht_handle_t symbol_table_t;

static symbol_table_t symbol_table;

static void destroy_symbol_table(void)
{
    ENTER();
    // TODO: destroy all attributes
    // TODO: add functions to destroy all of the symbol attribute types
    destroy_hash_table(symbol_table);
    RET();
}

void init_symbol_table(void)
{
    // the number 1223 is a prime number of sufficient size.
    symbol_table = create_hash_table(1223);
    atexit(destroy_symbol_table);
}

int add_symbol(const char *sym)
{
    sattr_table_t tab;

    if (NULL == (tab = (sattr_table_t)calloc(NUM_SYMBOL_ATTRS, sizeof(void *))))
        FATAL("cannot allocate memry for symbol attribute table");

    return hash_save(symbol_table, sym, tab);
}

int check_symbol(const char *sym)
{
    return (hash_find(symbol_table, sym) == NULL) ? 0 : 1;
}

void add_symbol_attr(const char *sym, sym_attr_t type, void *data, unsigned int size)
{
    sattr_table_t tab;
    void *ndat;

    tab = hash_find(symbol_table, sym);
    if (tab == NULL)
    {
        add_symbol(sym);
        tab = hash_find(symbol_table, sym);
    }

    if(NULL == (ndat = calloc(1, size)))
        FATAL("cannot allocate memory for symbol attribute");

    memcpy(ndat, data, size);
    tab[type] = ndat;
}

void *get_symbol_attr(const char *sym, sym_attr_t type)
{
    sattr_table_t tab;

    tab = hash_find(symbol_table, sym);
    if (tab != NULL)
        return tab[type]; // could be NULL
    else
        return NULL;
}
