/*
    Top level parse routine. This is the parser "main" routine.

    The top level file should already be open when this is called.
*/


#include "parse_inc.h"

void do_end_file(void)
{
    ENTER();
    INFO("END OF FILE");
    RET();
}

void do_end_input(void)
{
    ENTER();
    INFO("END OF INPUT");
    RET();
}

void parse(void)
{
    ENTER();
    int finished = 0;
    token_t tok;

    while (!finished)
    {
        tok = get_token();
        switch (tok)
        {
        case IMPORT_TOK:
            do_import();
            break;
        case SYMBOL_TOK:
            do_class();
            break;
        case END_OF_FILE:
            do_end_file();
            finished++;
            break;
        case END_OF_INPUT:
            do_end_input();
            finished++;
            break;
        default:
            syntax("expected an import or class definition, but got \"%s\"", token_to_msg(tok));
            finished++;
            break;
        }
    }
    RET();
}
