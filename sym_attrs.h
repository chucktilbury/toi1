#ifndef _SYM_ATTRS_H_
#define _SYM_ATTRS_H_

typedef enum {
    RESERVED_SYM_ATTR,
    SYMBOL_TYPE_ATTR,
    SYM_TYPEOF_ATTR,
    COMPLEX_TYPEOF_ATTR,
    SYMBOL_SCOPE_ATTR,
    SYMBOL_ASSIGMENT_EXPR_ATTR,
    NUM_SYMBOL_ATTRS,
} sym_attr_t;

typedef enum {
    CLASS_SYMBOL,
    FUNC_SYMBOL,
    CLASS_VAR_SYMBOL,
    VAR_SYMBOL,

    PUBLIC_SCOPE,
    PRIVATE_SCOPE,
    PROTECTED_SCOPE,

    TYPEOF_INT,
    TYPEOF_UINT,
    TYPEOF_FLOAT,
    TYPEOF_STR,
    TYPEOF_COMPLEX,

} sym_attr_val_t;

#endif /* _SYM_ATTRS_H_ */