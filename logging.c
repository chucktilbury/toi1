
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
#include "logging.h"

// TODO: add the ability to push and pop debug levels.

static FILE *outfp[4];
static int debug_level = 0; // no debugging messages
static const char *type_strs[] = {
    "DEBUG", // debug messages appear at level 2
    "ENTER", // enter and return appear at level 5
    "RETURN",
    "INFO", // info messages appear at level 1
    "WARN", // the rest of these always appear
    "ERROR",
    "SYNTAX",
    // internal and fatal end the program
    // internal errors are error conditions that should "never happen"
    "INTERNAL",
    // fatal errors are conditions such as not being able to open a file
    "FATAL",
};

void set_debug_level(int lev)
{
    debug_level = lev;
}

void write_buffer(const char *buffer, size_t blen)
{
    if (outfp[0] != NULL)
        fwrite(buffer, 1, blen, outfp[0]);
    if (outfp[1] != NULL)
        fwrite(buffer, 1, blen, outfp[1]);
}

void write_args(const char *str, va_list args)
{
    char buffer[1024];

    vsnprintf(buffer, sizeof(buffer), str, args);
    write_buffer(buffer, strlen(buffer));
}

void write_str(const char *str, ...)
{
    va_list args;
    char buffer[1024];

    va_start(args, str);
    vsnprintf(buffer, sizeof(buffer), str, args);
    va_end(args);

    write_buffer(buffer, strlen(buffer));
}

void show_verbose_start(int type, const char *func, int line)
{

    time_t ti;
    struct tm *t;

    time(&ti);
    t = localtime(&ti);

    write_str("[%02d/%02d/%d %02d:%02d:%02d] %s: %s: %d: ",
              t->tm_mon, t->tm_mday, t->tm_year + 1900,
              (t->tm_hour==0)?12:t->tm_hour,
              t->tm_min, t->tm_sec,
              type_strs[type], func, line);
}

void show_start(int type)
{
    write_str("%s: ", type_strs[type]);
}


void show_msg(typelog_t type, int lev, const char *func, int line, const char *fmt, ...)
{

    va_list args;

    if (type == DEBUG || type == ENTER || type == RETURN)
    {
        if (lev <= debug_level)
        {
            show_verbose_start(type, func, line);
            va_start(args, fmt);
            write_args(fmt, args);
            va_end(args);
            write_str("\n");
        }
    }
    else if (type == INTERNAL)
    {
        show_verbose_start(type, func, line);
        va_start(args, fmt);
        write_args(fmt, args);
        va_end(args);
        write_str("\n");
        exit(1);
    }
    else if (type == INFO)
    {
        if (lev <= debug_level)
        {
            show_start(type);
            va_start(args, fmt);
            write_str(fmt, args);
            va_end(args);
            write_str("\n");
        }
    }
    else
    {
        show_start(type);
        va_start(args, fmt);
        write_str(fmt, args);
        va_end(args);
        write_str("\n");
        if (type == FATAL)
            exit(1);
    }
}

void clean_log_file(void)
{
#ifdef _DEBUGGING
    show_verbose_start(DEBUG, __func__, __LINE__);
    write_str("closing log file\n");
#endif
    if (outfp[1] != NULL)
        fclose(outfp[1]);
}

void init_logging(logging_flags_t flags, ...)
{
    va_list args;

    memset(outfp, 0, sizeof(outfp));

    if (flags & LOG_STDOUT)
        outfp[0] = stdout;

    // if both stdout and stderr are selected, then stderr is used.
    if (flags & LOG_STDERR)
        outfp[0] = stderr;

    if (flags & LOG_FILE)
    {
        va_start(args, flags);
        char *fname = va_arg(args, char *);
        va_end(args);
        char *mode;
        if (flags & LOG_APPEND)
            mode = "a";
        else
            mode = "w";

        if (NULL == (outfp[1] = fopen(fname, mode)))
        {
            fprintf(stderr, "FATAL ERROR: Cannot open the logfile \"%s\": %s\n", fname, strerror(errno));
            exit(1);
        }
        atexit(clean_log_file);
    }
}
