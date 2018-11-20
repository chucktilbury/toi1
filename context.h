#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include <stdint.h>

void init_context(void);
const char *push_context(const char *symb);
const char *push_anon_context(void);
const char *pop_context(void);
const char *make_context(const char *symb);
const char *make_anon_context(const char *symb);
const char *get_context(void);
uint64_t hash(char *str);

#endif /* _CONTEXT_H_ */