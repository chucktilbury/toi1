#ifndef _SYMBOLS_H_
#define _SYMBOLS_H_

typedef enum
{
    RESERVED_SYM_ATTR,
    NUM_SYMBOL_ATTRS
} sym_attr_t;

void init_symbol_table(void);
int add_symbol(const char *sym);
void add_symbol_attr(const char *sym, sym_attr_t type, void *data);
int check_symbol(const char *sym);
void *get_symbol_attr(const char *sym, sym_attr_t type);

#endif /* _SYMBOLS_H_ */