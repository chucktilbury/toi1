/*
    The putpose of this file is to display syntax-like errors and warnings.
*/
#include <stdio.h>
#include <stdarg.h>
#include "logging.h"
#include "file_io.h"
#include "scanner.h"

typedef struct tmgs
{
    const char *short_msg;
    const char *long_msg;
} tok_msg_t;

tok_msg_t tok_msg[] = {
    // NOTE that the order of these elements MUST be the same as they are
    // in the scanner.h definition.
    {/* FIRST_TOK, */ NULL, NULL},
    {/* ERROR_TOK, */ "error token", NULL},
    {/* END_OF_FILE, */ "end of file", NULL},
    {/* END_OF_INPUT, */ "end of input", NULL},
    {/* SYMBOL_TOK, */ "symbol token", NULL},
    {/* UINT_TOK, */ "unsigned number literal", NULL},
    {/* INT_TOK, */ "signed integer literal", NULL},
    {/* FLOAT_TOK, */ "floating point literal", NULL},
    {/* STRING_TOK, */ "quoted string literal", NULL},
    {/* IMPORT_TOK, */ "import", "import another module"},
    {/* CLASS_TOK, */ "class", "introduce a definition of a class"},
    {/* FUNC_TOK, */ "func or function", "introduce a definition of a function"},
    {/* UINT_DEF_TOK, */ "uint or unsigned", "introduce a definition of type unsigned integer"},
    {/* INT_DEF_TOK, */ "int or integer", "introduce a definition of type integer"},
    {/* FLOAT_DEF_TOK, */ "float", "introduce a definition of type float"},
    {/* STR_DEF_TOK, */ "str or string", "introduce a definition of type string"},
    {/* IF_TOK, */ "if", "introduce an if clause"},
    {/* ELSE_TOK, */ "else", "introduce an else clause for an if or a switch"},
    {/* SWITCH_TOK, */ "switch", "introduce a switch clause"},
    {/* CASE_TOK, */ "case", "introduce a case clause for a switch"},
    {/* TRY_TOK, */ "try", "watch for an exception"},
    {/* EXCEPT_TOK, */ "except or exception", "catch an exception"},
    {/* WHILE_TOK, */ "while", "introduce a while loop"},
    {/* FOR_TOK, */ "for", "introduce a for loop"},
    {/* CONTINUE_TOK, */ "cont or continue", "continue to top of loop"},
    {/* BREAK_TOK, */ "break", "break out of loop"},
    {/* RETURN_TOK, */ "return", "return from function"},
    {/* PRIVATE_TOK, */ "private", "scope operator"},
    {/* PUBLIC_TOK, */ "public", "scope operator"},
    {/* PROTECTED_TOK, */ "protected", "scope operator"},
    {/* EQUAL_TOK, */ "== or eq", "greater comparison operator"},
    {/* NEQUAL_TOK, */ "!= or neq", "not equal comparison operator"},
    {/* LESS_TOK, */ "<< or lt", "less than comparison operator"},
    {/* GREATER_TOK, */ ">> or gt", "greater than comparison operator"},
    {/* LESS_OR_EQUAL_TOK, */ "<= or leq", "less or equal comparison operator"},
    {/* GREATER_OR_EQUAL_TOK, */ ">= or geq", "greater or equal comparison operator"},
    {/* AND_TOK, */ "&& or and", "AND comparison operator"},
    {/* OR_TOK, */ "|| or \'or\'", "OR comparison operator"},
    {/* NOT_TOK, */ "!! or not or !", "negation comparison operator"},
    {/* PLUS_TOK, */ "+", "addition arithmetic operator"},
    {/* INCREMENT_TOK, */ "++", "increment operator"},
    {/* DECREMENT_TOK, */ "--", "decrement operator"},
    {/* MINUS_TOK, */ "-", "subtraction arithmetic operator"},
    {/* MUL_TOK, */ "*", "multiplication arithmetic operator"},
    {/* DIV_TOK, */ "/", "division arithmetic operator"},
    {/* MOD_TOK, */ "%", "modulo arithmetic operator"},
    {/* LOR_TOK, */ "|", "binary OR operator"},
    {/* LAND_TOK, */ "&", "binary AND operator"},
    {/* LXOR_TOK, */ "^", "binary XOR operator"},
    {/* LSHR_TOK, */ ">", "binary right shift operator"},
    {/* LSHL_TOK, */ "<", "binary left shift operator"},
    {/* LNOT_TOK, */ "~", "binary NOT operator"},
    {/* OPAREN_TOK, */ "(", "start of a tuple"},
    {/* CPAREN_TOK, */ ")", "end of a tuple"},
    {/* OCURLY_TOK, */ "{", "start of code block"},
    {/* CCURLY_TOK, */ "}", "end of code block"},
    {/* OSQU_TOK, */ "[", "start of list"},
    {/* CSQU_TOK, */ "]", "end of list"},
    {/* SEMI_TOK, */ ";", "statement end"},
    {/* COLON_TOK, */ ":", "declaration operator"},
    {/* COMMA_TOK, */ ",", "separator"},
    {/* DOT_TOK, */ ".", "scope operator"},
    {/* ASSIGN_TOK, */ "=", "assignment"},
    {/* CREATE_TOK, */ "create", "class constructor"},
    {/* DESTROY_TOK, */ "destroy", "class destructor"},
    {/* NIL_TOK, */ "nothing or nill or null", NULL},
    {/* TRUE_TOK, */ "true or yes", NULL},
    {/* FALSE_TOK, */ "false or no", NULL},
    {/* RESERVED_TOK, */ "RESERVED_TOK", NULL},
    {/* LAST_TOKEN, */ NULL, NULL}};
const int msg_size = sizeof(tok_msg) / sizeof(tok_msg_t);

void warning(const char *fmt, ...)
{
    va_list args;
    fprintf(stdout, "Warning: %s: %d: %d: ", file_name(), line_number(), line_index());
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fprintf(stdout, "\n");
}

void syntax(const char *fmt, ...)
{
    va_list args;
    fprintf(stdout, "Syntax: %s: %d: %d: ", file_name(), line_number(), line_index());
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fprintf(stdout, "\n");
}

const char *token_to_str(token_t tok)
{
    int index = tok - FIRST_TOK;
    if (index > 0 && index < msg_size)
        return tok_msg[index].short_msg;
    else
        return NULL;
}

const char *token_to_msg(token_t tok)
{
    char *str = NULL;
    int index = tok - FIRST_TOK;

    if (index > 0 && index < msg_size)
    {
        if (tok_msg[index].long_msg != NULL)
            str = (char *)tok_msg[index].long_msg;
        else
            str = (char *)tok_msg[index].short_msg;
    }
    return str;
}

void expect_token(token_t expect, token_t got, const char *msg)
{
    if (expect != got)
    {
        if (NULL != msg)
            syntax("expected a %s but got a %s. %s.", token_to_msg(expect), token_to_msg(got), msg);
        else
            syntax("expected a %s but got a %s", token_to_msg(expect), token_to_msg(got));
    }
}
