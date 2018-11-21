/*
    Main program entry.
*/

#include "toi.h"

static void init_toi(const char* fname) {

    init_logging(LOG_STDOUT);
    init_file_io();
    init_scanner(fname);
    init_context();
}

int main(void)
{
    init_toi("tests/scanner1.txt");
    return 0;
}