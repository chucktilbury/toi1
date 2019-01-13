/*
    Handle a class definition.
*/
#include <stdio.h>
#include <string.h>

#include "logging.h"
#include "errors.h"
#include "scanner.h"
#include "context.h"
#include "symbols.h"
#include "file_io.h"
#include "sym_attrs.h"

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
static void get_func_def(void)
{
    INFO("function definition: %s", "func");
}

/*
 * Note that the void pointer needs to point to something that is
 * memcpy()-able. Bsically, this reads a list of symbols seprated by '.' and
 * returns it exactly as read.
 *
 * The name parameter is the first item in the symbol and it's volitile to the
 * scanner.
 */
static void *get_complex_type(const char *name, unsigned int *size) {

    // TODO: semantics: Verify that the name given is accessible and that it's
    // defined as a class.
    static char buff[1024]; // ugly....
    int finished = 0;
    int state = 0; // entry state is expecting to see a '.'
    token_t tok;

    memset(buff, 0, sizeof(buff));
    strcpy(buff, name);

    while(!finished) {
        tok = get_token();
        switch(state) {
            case 0:
                if(tok == DOT_TOK) {
                    strcat(buff, ".");
                    state = 1;
                }
                else {
                    unget_token();
                    finished++;
                }
                break;
            case 1:
                if(tok == SYMBOL_TOK) {
                    strcat(buff, get_token_string());
                    state = 0;
                }
                else {
                    unget_token();
                    finished++;
                }
                break;
            default:
                // should never happen
                FATAL("invalid state in get_complex_type()");
                return NULL;
        }
    }

    INFO("complex symbol: %s", buff);
    *size = strlen(buff)+1;
    return (void*)buff;
}

static void* get_class_var_assignment(unsigned int* size) {
    // TODO: call the expression parser
    token_t tok;
    char *estr;

    tok = get_token();
    estr = strdup(get_token_string());
    *size = strlen(estr)+1;
    INFO("assignment var: %s, %d", estr, *size);
    return (void*)estr;
}
/*
 * For class var defs the assignment expression is optional, unlike local
 * variable definitions.
 *
 * A var def has the form
 * var name:type:scope = expression;
 *
 * The scope and expression are optional for class variables. A class variable
 * must be assigned in the constructor if it is not assigned in the
 * declaration.
 */
static void get_class_var_def(void)
{
    token_t tok;
    const char *name;
    sym_attr_val_t attr = CLASS_VAR_SYMBOL;
    int state = 0;
    int finished = 0;
    void* str;
    unsigned int size;

    INFO("getting class var symbol");

    // get the symbol name, mandtory
    tok = get_token();
    if(tok != SYMBOL_TOK) {
        syntax("expected name of a variable but got %s", token_to_msg(tok));
        RET(); // restart parsing
    }

    // TODO: symantics: Make sure that this symbol name does not already exist
    // in this context.

    name = strdup(get_token_string());
    add_symbol(name);
    add_symbol_attr(name, SYMBOL_TYPE_ATTR, (void*)&attr, sizeof(sym_attr_val_t));
    INFO("class var symbol name is %s", name);

    // get the symbol type specifier, type is mandatory.
    tok = get_token();
    if(tok != COLON_TOK) {
        syntax("expected :type but got %s", token_to_msg(tok));
        RET(); // restart parsing
    }

    tok = get_token();
    switch(tok) {
        case UINT_DEF_TOK:
            INFO("var type is unsigned int");
            attr = TYPEOF_UINT;
            add_symbol_attr(name, SYM_TYPEOF_ATTR, (void*)&attr, sizeof(sym_attr_val_t));
            break;
        case INT_DEF_TOK:
            INFO("var type is signed int");
            attr = TYPEOF_INT;
            add_symbol_attr(name, SYM_TYPEOF_ATTR, (void*)&attr, sizeof(sym_attr_val_t));
            break;
        case FLOAT_DEF_TOK:
            INFO("var type is float");
            attr = TYPEOF_FLOAT;
            add_symbol_attr(name, SYM_TYPEOF_ATTR, (void*)&attr, sizeof(sym_attr_val_t));
            break;
        case STR_DEF_TOK:
            INFO("var type is string");
            attr = TYPEOF_STR;
            add_symbol_attr(name, SYM_TYPEOF_ATTR, (void*)&attr, sizeof(sym_attr_val_t));
            break;
        case SYMBOL_TOK:
            INFO("var type is complex symbol");
            attr = TYPEOF_COMPLEX;
            add_symbol_attr(name, SYM_TYPEOF_ATTR, (void*)&attr, sizeof(sym_attr_val_t));
            unsigned int size = 0;
            add_symbol_attr(name, COMPLEX_TYPEOF_ATTR,
                            get_complex_type(get_token_string(), &size), size);
            break;
        default:
            syntax("expected type definition but got %s", token_to_msg(tok));
            return;
    }

    // get the symbol scope and/or assignment, optional.
    while(!finished) {
        tok = get_token();
        switch(state) {
            case 0:
                // could be a ':', '=', or ';'
                switch(tok) {
                    case SEMI_TOK:
                        INFO("var has no scope operator or assignment, is PRIVATE scope");
                        attr = PRIVATE_SCOPE;
                        add_symbol_attr(name, SYMBOL_SCOPE_ATTR, (void*)&attr, sizeof(sym_attr_val_t));
                        finished++;
                        break;
                    case ASSIGN_TOK:
                        INFO("var has no scope operator, is PRIVATE scope");
                        attr = PRIVATE_SCOPE;
                        add_symbol_attr(name, SYMBOL_SCOPE_ATTR, (void*)&attr, sizeof(sym_attr_val_t));
                        str = get_class_var_assignment(&size);
                        add_symbol_attr(name, SYMBOL_ASSIGMENT_EXPR_ATTR, str, size);
                        state = 5;
                        break;
                    case COLON_TOK:
                        state = 2;
                        break;
                    default:
                        syntax("expected scope, assignment or statement end, but got %s", token_to_msg(tok));
                }
                break;
            case 1:
                // statement end
                finished++;
                break;
            case 2:
                // get the scope operator
                switch(tok) {
                    case PUBLIC_TOK:
                        INFO("var is PUBLIC scope");
                        attr = PUBLIC_SCOPE;
                        add_symbol_attr(name, SYMBOL_SCOPE_ATTR, (void*)&attr, sizeof(sym_attr_val_t));
                        break;
                    case PRIVATE_TOK:
                        INFO("var is PRIVATE scope");
                        attr = PRIVATE_SCOPE;
                        add_symbol_attr(name, SYMBOL_SCOPE_ATTR, (void*)&attr, sizeof(sym_attr_val_t));
                        break;
                    case PROTECTED_TOK:
                        INFO("var is PROTECTED scope");
                        attr = PROTECTED_SCOPE;
                        add_symbol_attr(name, SYMBOL_SCOPE_ATTR, (void*)&attr, sizeof(sym_attr_val_t));
                        break;
                    default:
                        syntax("expected a scope operator but got %s", token_to_msg(tok));
                        RET();
                }
                state = 3;
                break;
            case 3:
                // have the scope operator, could be an assignement or statement end
                switch(tok) {
                    case SEMI_TOK:
                        INFO("var has no assignment");
                        finished++;
                        break;
                    case ASSIGN_TOK:
                        str = get_class_var_assignment(&size);
                        add_symbol_attr(name, SYMBOL_ASSIGMENT_EXPR_ATTR, str, size);
                        state = 5;
                        break;
                    default:
                        syntax("expected assignment or statement end, but got %s", token_to_msg(tok));
                        RET();
                }
                break;
            case 5:
                if(tok != SEMI_TOK)
                    syntax("expected end of statement but got %s", token_to_msg(tok));
                else
                    finished++;
                break;
            default:
                FATAL("invalid state in get_class_var(): %d", state);
        }
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
        switch (tok) {
            case CCURLY_TOK:
                finished++;
                break;

            case VAR_TOK:
                get_class_var_def();
                break;

            case FUNC_TOK:
                get_func_def();
                break;

            case END_OF_FILE:
            case END_OF_INPUT:
                syntax("unexpected end of input in class definition");
                RET();

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
    // TODO: semantics: Verify that the symbol is defined as a class and that
    // it's acssesible. Add the specified class's richness to this class as a
    // stand-alone class.
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
    When this is entered, a class token has been encountered. Parse the class
    declaration and the body of the class.

    The class definition has the form of
    name(inherit1, inherit2, ...):scope {class body}
    name(inherit1, inherit2, ...) {class body}

    The inherited classes must already have a definition. The class body
    consists of variable definitions and function definitions.
*/
void do_class(void)
{
    token_t tok;
    const char* str;
    sym_attr_val_t attr = CLASS_SYMBOL;

    ENTER();

    tok = get_token();
    if(tok != SYMBOL_TOK) {
        syntax("expected name of class but got %s", token_to_msg(tok));
        RET(); // restart parsing
    }

    str = strdup(get_token_string());
    push_context(str);
    add_symbol(str);
    add_symbol_attr(str, SYMBOL_TYPE_ATTR, (void*)&attr, sizeof(sym_attr_val_t));

    // TODO: semantics: check for duplicate class names and perform
    // redefinition if needed.

    tok = get_token();
    switch(tok) {
        case COLON_TOK:
            tok = get_token();
            if(tok == PUBLIC_TOK) {
                // store the attrib in the symbol
                INFO("class is PUBLIC scope");
                attr = PUBLIC_SCOPE;
                add_symbol_attr(str, SYMBOL_SCOPE_ATTR, (void*)&attr, sizeof(sym_attr_val_t));
            }
            else if(tok == PRIVATE_TOK) {
                // store the attrib
                INFO("class is PRIVATE scope");
                attr = PRIVATE_SCOPE;
                add_symbol_attr(str, SYMBOL_SCOPE_ATTR, (void*)&attr, sizeof(sym_attr_val_t));
            }
            else {
                syntax("expected a scope operator but got %s", token_to_msg(tok));
                pop_context();
                free(str);
                RET(); // restart paring
            }
            get_class_parameters();
            get_class_body();
            break;

        case OPAREN_TOK:
            // save the scope as private
            INFO("no scope operator, scope is PRIVATE");
            attr = PRIVATE_SCOPE;
            add_symbol_attr(str, SYMBOL_SCOPE_ATTR, (void*)&attr, sizeof(sym_attr_val_t));
            unget_char('(');
            get_class_parameters();
            get_class_body();
            break;
        case END_OF_FILE:
        case END_OF_INPUT:
            syntax("unexpected end of file");
            break;
        default:
            syntax("expected class parameters or scope operator but got %s", token_to_msg(tok));
            break;
    }

    pop_context();
    free(str);
    RET();
}

