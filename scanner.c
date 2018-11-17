/*
    Return tokens from the input stream. All tokens are returned as symbolic
    values. The function get_token_string() returns the string associated
    with the last call to get_token(). The other functions in the public
    interface depend on having already called get_token() as well.

    This module wraps functionality from file_io to provide a consistent
    public interface to error handlers.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "logging.h"
#include "file_io.h"
#include "scanner.h"
/*
    Internally, the scanner is just a big ugly state machine
*/

/*
    Public interface
*/
void init_scanner(const char *fname)
{
    // init the file_io
    // set up the scanner's internal tables
}

int get_token(void)
{

}

const char *get_token_string(void)
{

}

int get_token_index(void)
{

}

int get_token_line(void)
{

}

const char *get_token_file(void)
{

}

const char *token_to_str(token_t tok)
{

}
