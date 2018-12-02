/*
    Handle imports. 

    When do_import is called an "import" token has been scanned. The tokens 
    that follow is a single symbol that is reduced to a file to read. There
    is a system path that is used to find the files, similar to the way python 
    handles imports. The string ".toi" is appended and the file is opened for 
    scanning. A new context is also pushed (without the file extention) and the 
    symbols in the file are accessed using the context.

    Example:
    import file1;
*/
#include <stdio.h>
#include <string.h>

#include "logging.h"
#include "errors.h"
#include "scanner.h"
#include "parse.h"
#include "context.h"
#include "file_io.h"

#define FNAME_SIZE 1024

static void decorate_import(const char *token, char *buf)
{
    // stubbed. just a canned file for testing.
    strcpy(buf, "tests/");
    strcat(buf, token);
    strcat(buf, ".toi");
}

void do_import(void)
{
    ENTER();
    token_t tok;
    char fname[FNAME_SIZE];

    tok = get_token();
    if (tok == SYMBOL_TOK)
    {
        push_context(get_token_string());
        INFO("context: %s", get_context());
        decorate_import(get_token_string(), fname);
        open_file(fname);
        parse();

        tok = get_token();
        if (tok != SEMI_TOK)
            expect_token(SEMI_TOK, tok, NULL);
    }
    else
        expect_token(SYMBOL_TOK, tok, "import failed");

    RET();
}
