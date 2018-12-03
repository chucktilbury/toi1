/*
    Handle a class definition.
*/

#include <string.h>

#include "logging.h"
#include "errors.h"
#include "scanner.h"
#include "context.h"
#include "symbols.h"

/*
    The next token should be the name of the var to define.
*/
static void get_strg_def(void)
{
    token_t tok;
}

/*
    The next token should be the name of the var to define.
*/
static void get_float_def(void)
{
}

/*
    The next token should be the name of the var to define.
*/
static void get_int_def(void)
{
}

/*
    The next token should be the name of the var to define.
*/
static void get_uint_def(void)
{
}

/*
    The next token should be the name of the var to define.
*/
static void get_object_def(const char *name)
{
}

/*
    When this is called, the open paren for the parameter tuples has already 
    been seen.
*/
static void get_func_def(const char *name)
{
}

/*
    For class var defs the assignment expression is optional, unlike local
    variable definitions.

    A var def has the form of int:name = expression; where the assignment 
    is optional.

    A function def has the form name(list)(list){body}
*/
static void get_class_var_def(token_t t)
{
    token_t tok;
    const char *name = strdup(get_token_string());

    tok = get_token();
    switch (tok)
    {
    case COLON_TOK: // variable
        switch (t)
        {
        case SYMBOL_TOK:
            get_object_def(name);
            break;
        case UINT_DEF_TOK:
            get_uint_def();
            break;
        case INT_DEF_TOK:
            get_int_def();
            break;
        case FLOAT_DEF_TOK:
            get_float_def();
            break;
        case STR_DEF_TOK:
            get_strg_def();
            break;
        default:
            break;
        }
        break;
    case OPAREN_TOK: // function
        get_func_def(name);
        break;
    default:
        syntax("expected a variable or function definition, but got %s", token_to_msg(tok));
        break;
    }

    free(name);
}

/*
    The class body can have variable definitions and function definitions.
    Class definitions can be empty.
*/
static void get_class_body(void)
{
    token_t tok;
    int finished = 0;
    int state = 0;

    tok = get_token();
    if (tok != OCURLY_TOK)
        expect_token(OCURLY_TOK, tok, "");

    while (!finished)
    {
        tok = get_token();
        switch (tok)
        {
        case CCURLY_TOK:
            finished++;
            break;
        case SYMBOL_TOK: // a function def or a class instance
        case UINT_DEF_TOK:
        case INT_DEF_TOK:
        case FLOAT_DEF_TOK:
        case STR_DEF_TOK:
            get_class_var_def(tok);
            break;
        default:
            syntax("expected a function or a variable definition but got %s", token_to_msg(tok));
            break;
        }
    }
}

/*
    Add the symbols currently defined for this class name to 
    the definition of the current class.
*/
static void get_inheritance_class(const char *name)
{
    INFO("getting class %s for inheritance", name);
}

static void get_class_parameters(void)
{
    token_t tok;
    int finished = 0;
    int state = 0;

    tok = get_token();
    if (tok != OPAREN_TOK)
        expect_token(OPAREN_TOK, tok, "");

    while (!finished)
    {
        tok = get_token();
        switch (state)
        {
        case 0:
            switch (tok)
            {
            case SYMBOL_TOK:
                get_inheritance_class(get_token_string());
                state = 1;
                break;
            case CPAREN_TOK:
                INFO("empty class parameter list");
                finished++;
                break;
            default:
                syntax("expected a symbol or a close paren but got %s", token_to_msg(tok));
                finished++;
                break;
            }
            break;
        case 1:
            switch (tok)
            {
            case COMMA_TOK:
                state = 0;
                break;
            case CPAREN_TOK:
                finished++;
                break;
            default:
                syntax("expected a comma or a close paren but got %s", token_to_msg(tok));
                finished++;
                break;
            }
            break;
        default:
            syntax("expected a symbol or a close paren but got %s", token_to_msg(tok));
            finished++;
            break;
        }
    }
}

/*
    When this is entered, a name has been encountered that gived the name of 
    the class is being defined. Before any other actions are taken the name 
    needs be retreived and the context pushed.

    The class definition has the form of
    name(inherit1, inherit2, ...) {class body}

    The inherited classes must already have a definition. The class body 
    consists of variable definitions and function definitions.
*/
void do_class(void)
{
    ENTER();
    push_context(get_token_string());
    // Class names do not go into the symbol table because they are not
    // referred to directly, but in terms of the context they create.
    get_class_parameters();
    get_class_body();

    pop_context();
    RET();
}

// classes can have a scope operator?