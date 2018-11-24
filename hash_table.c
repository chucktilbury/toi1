/*
    Generic hash table to support the symbol table and symbol attributes.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "logging.h"
#include "errors.h"
#include "hash_table.h"
#include "xxhash.h"

typedef struct __hte__ {
    char *key;
    void *data;
    struct __hte__ *next;
} hash_table_entry_t;

typedef struct __htt__ {
    hash_table_entry_t **table;
    int slots;
} hash_table_t;


ht_handle_t create_hash_table(int slots)
{
    hash_table_t *ht;
    ENTER();
    if(NULL == (ht = (hash_table_t *)calloc(1, sizeof(hash_table_t)))) 
        FATAL("cannot allocate hash table struct");

    if(NULL == (ht->table = (hash_table_entry_t **)calloc(slots, sizeof(hash_table_entry_t)))) 
        FATAL("cannot allocate hash table");

    ht->slots = slots;
    VRET((ht_handle_t)ht);
}

void destroy_hash_table(ht_handle_t ht)
{
    ENTER();
    hash_table_entry_t *next, *hte;
    hash_table_t* table = (hash_table_t*)ht;

    if(NULL != table) {
        int i;
        for(i = 0; i < table->slots; i++) {
            if(NULL != table->table[i]) {
                for(hte = table->table[i]; hte != NULL; hte = next) {
                    next = hte->next;
                    if(NULL != hte->key) {
                        free(hte->key);
                    }

                    free(hte);
                }
            }
        }
    }
    RET();
}

uint32_t make_hash(const char *str)
{
    return (uint32_t)XXH32(str, strlen(str), -1);
}

static inline hash_table_entry_t *find_local(hash_table_t *table, const char *key, int hash) {

    hash_table_entry_t *hte;

    for(hte = table->table[hash]; NULL != hte; hte = hte->next) {
        if(!strcmp(key, hte->key)) {
            return hte; // found
        }
    }
    return NULL;    // not found
}

int hash_save(ht_handle_t ht, const char *key, void *data)
{
    hash_table_t* table = (hash_table_t*)ht;
    hash_table_entry_t *hte;
    int hash;

    if(NULL != table) {
        hash = make_hash(key) % table->slots;
        hte = find_local(table, key, hash);
        if(NULL == hte) {
            if(NULL == (hte = (hash_table_entry_t *)calloc(1, sizeof(hash_table_entry_t)))) 
                FATAL("cannot allocate table entry");

            if(NULL == (hte->key = strdup(key))) 
                FATAL("cannot allocate table entry name");

            hte->data = data;
            hte->next = NULL;

            if(NULL != table->table[hash])
                hte->next = table->table[hash];
            table->table[hash] = hte;

            return 0;   // success
        }
        else {
            return 1;   // symbol already in the table
        }
    }
    else {
        return -1;      // table is invalid
    }
}

void *hash_find(ht_handle_t ht, const char *key)
{
    hash_table_t* table = (hash_table_t*)ht;
    hash_table_entry_t *hte;

    if(NULL != table) {
        hte = find_local(table, key, make_hash(key) % table->slots);
        if(NULL != hte)
            return hte->data;  // success
        else
            return NULL;    // not found
    }
    else
        return NULL;    // invalid table
}

