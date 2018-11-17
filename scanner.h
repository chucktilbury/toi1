#ifndef _SCANNER_H_
#define _SCANNER_H_

typedef enum
{
    ERROR_TOK = 2000, // returned when the scanner can't scan the input for semantic reasons.
    END_OF_FILE,    // lets the parser know that a context change is happening
    END_OF_INPUT,   // signals the end of all input
    // constructed tokens, literals
    SYMBOL_TOK, // [a-zA-Z_$]+[a-zA-Z_$0-9]*
    UINT_TOK,   // 0[xX][0-9a-fA-F]+
    INT_TOK,    // [-+]?[0-9]+
    FLOAT_TOK,  // [-+]?[0-9]+\.[0-9]+([eE][-+]?[0-9]+)?
    STRING_TOK, // single quoted string are absolute literal, double quoted string can have escapes.
    // scope definitions
    IMPORT_TOK, // "import"
    CLASS_TOK,  // "class"
    FUNC_TOK,   // "func"
    // native types
    UINT_DEF_TOK,  // "uint" | "unsigned"
    INT_DEF_TOK,   // "int" | "integer"
    FLOAT_DEF_TOK, // "float"
    STR_DEF_TOK,   // "str" | "string"
    // flow control
    IF_TOK,       // "if"
    ELSE_TOK,     // "else"
    SWITCH_TOK,   // "switch"
    CASE_TOK,     // "case"
    TRY_TOK,      // "try"
    EXCEPT_TOK,   // "except" | "exception"
    WHILE_TOK,    // "while"
    FOR_TOK,      // "for"
    CONTINUE_TOK, // "cont" | "continue"
    BREAK_TOK,    // "break"
    RETURN_TOK,   // "return"
    // scope operators
    PRIVATE_TOK,   // "private"
    PUBLIC_TOK,    // "public"
    PROTECTED_TOK, // "protected"
    // comparison operators
    EQUAL_TOK,            // "==" | "eq"
    NEQUAL_TOK,           // "!=" | "neq"
    LESS_TOK,             // "<<" | "lt"
    GREATER_TOK,          // ">>" | "gt"
    LESS_OR_EQUAL_TOK,    // "<=" | "geq"
    GREATER_OR_EQUAL_TOK, // ">=" | "leq"
    AND_TOK,              // "&&" | "and"
    OR_TOK,               // "||" | "or"
    NOT_TOK,              // "!!" | "not" | "!"
    // arithmetic operators
    PLUS_TOK,  // "+"
    MINUS_TOK, // "-"
    MUL_TOK,   // "*"
    DIV_TOK,   // "/"
    MOD_TOK,   // "%"
    // bitwise operators
    LOR_TOK,  // "|"
    LAND_TOK, // "&"
    LXOR_TOK, // "^"
    LSHR_TOK, // ">"
    LSHL_TOK, // "<"
    LNOT_TOK, // "~"
    // syntax support tokens
    OPAREN_TOK, // "("
    CPAREN_TOK, // ")"
    OCURLY_TOK, // "{"
    CCURLY_TOK, // "}"
    OSQU_TOK,   // "["
    CSQU_TOK,   // "]"
    SEMI_TOK,   // ";"
    COLON_TOK,  // ":"
    COMMA_TOK,  // ","
    DOT_TOK,    // "."
    ASSIGN_TOK, // "="
    // misc tokens
    CREATE_TOK,  // "create"
    DESTROY_TOK, // "destroy"
    NIL_TOK,     // "nothing" | "nill" | "null"
    TRUE_TOK,    // "true" | "yes"
    FALSE_TOK,   // "false" | "no"
    // reserved characters, not allowed in any token
    RESERVED_TOK, // "?`@#\\" The "#" is used for comments. other chars are allowed in strings.
    // just marks the highest number defined
    LAST_TOKEN
} token_t;

// must be called before any other scanner function
void init_scanner(const char *fname);

// These functions are used mostly by the parser
int get_token(void);
const char *get_token_string(void);

// these functions are used mostly for error handling
int get_token_index(void);
int get_token_line(void);
const char *get_token_file(void);
const char *token_to_str(token_t tok);

#endif /* _SCANNER_H_ */