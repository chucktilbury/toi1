/*
    Main program entry.
*/

#include "toi.h"

static void init_toi(const char* fname) {

    init_logging(LOG_STDOUT);
    set_debug_level(10);
    init_scanner(fname);
    init_context();
    init_symbol_table();
}

int main(void)
{
    init_toi("tests/scanner1.txt");
    return 0;
}