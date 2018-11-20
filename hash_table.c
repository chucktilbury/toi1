/*
    Generic hash table to support the symbol table and symbol attributes.
*/
#include <stdio.h>
#include <stdlib.h>

#include "logging.h"
#include "errors.h"
#include "context.h" // for the hash function
#include "hash_table.h"

typedef struct hash_table_entry_t
{
    char *key;
    void *data;
    struct hash_table_entry_t *next;
} hash_table_entry_t;

typedef struct hash_table_t
{
    int size;
    hash_table_entry_t *hash_array;
} hash_table_t;

ht_handle_t create_hash_table(int slots)
{
}

void destroy_hash_table(ht_handle_t ht)
{
}

int hash_save(ht_handle_t ht, const char *key, void *data)
{
}

void *hash_find(ht_handle_t ht, const char *key)
{
}
