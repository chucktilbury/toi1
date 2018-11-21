#ifndef _HASH_TABLE_H_
#define _HASH_TABLE_H_

#include <stdint.h>

typedef void *ht_handle_t;

ht_handle_t create_hash_table(int slots);
void destroy_hash_table(ht_handle_t ht);
int hash_save(ht_handle_t ht, const char *key, void *data);
void *hash_find(ht_handle_t ht, const char *key);
uint32_t make_hash(const char *str);

#endif /* _HASH_TABLE_H_ */