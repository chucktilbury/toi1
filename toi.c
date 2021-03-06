/*
    Main program entry.
*/

#include "toi.h"

static void init_toi(const char* fname) {

    init_logging(LOG_STDOUT);
    set_debug_level(7);
    init_scanner(fname);
    init_context();
    init_symbol_table();
}

int main(void)
{
    init_toi("tests/parse1.txt");
    parse();
    return 0;
}
