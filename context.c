/*
    Manipulate the symbolic context of a word. Words are stored such that their 
    name describes where the symbol was defined so that a single global symbol 
    table can be maintained. 

    For example, if a class has a method named foo, and the class is named bar, 
    then the context would be @bar@foo. Function names are decorated with 
    their parameters as well, but not by these functions.
*/
#include <stdio.h>
#include <string.h>

#include "logging.h"
#include "errors.h"
#include "context.h"
#include "hash_table.h"

#define CONTEXT_SIZE 1024 * 2
#define PAD 2

static char context[CONTEXT_SIZE];
static char temp_context[CONTEXT_SIZE];

void init_context(void)
{
    ENTER();
    memset(context, 0, CONTEXT_SIZE);
    context[0] = '@';
    RET();
}

const char *push_context(const char *symb)
{
    ENTER();
    if (strlen(symb) + strlen(context) + PAD >= CONTEXT_SIZE)
        FATAL("symbol context buffer overrun");

    strcat(context, "@");
    strcat(context, symb);
    VRET(context);
}

const char *pop_context(void)
{
    ENTER();
    char *ch = strrchr(context, '@');
    if (ch == NULL)
        INTERNAL("symbol context is empty or corrupt");

    if (strlen(context) == 0)
        INTERNAL("symbol context is empty");

    *ch = 0;
    VRET(context);
}

/*
    create a context without saving it. This is used for things like local 
    vars.
*/
const char *make_context(const char *symb)
{
    ENTER();
    if (strlen(symb) + strlen(context) + PAD >= CONTEXT_SIZE)
        FATAL("temp symbol context buffer overrun");

    strcpy(temp_context, context);
    strcat(temp_context, "@");
    strcat(temp_context, symb);
    VRET(temp_context);
}

/*
    An anonymous context is used where there is no name to tie it to, such 
    as in a while() loop.
*/
const char *push_anon_context(void)
{
    ENTER();
    uint32_t h = make_hash(context);
    char buf[17];

    sprintf(buf, "%08X", h);
    VRET(push_context(buf));
}

const char *get_context(void)
{
    ENTER();
    VRET(context);
}

#ifdef _TESTING

int main(void)
{
    init_logging(LOG_STDOUT);
    set_debug_level(10);
    init_context();
    printf("%s\n", get_context());
    push_context("asdf");
    push_context("qwerty");
    printf("%s\n", get_context());
    push_anon_context();
    push_context("oiuoiu");
    printf("%s\n", get_context());
    pop_context();
    push_context("lkjlkj");
    printf("%s\n", get_context());
    pop_context();
    pop_context();
    printf("%s\n", get_context());
    return 0;
}

#endif