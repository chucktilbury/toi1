
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "logging.h"

// TODO:
// (BUG) Current file needs to remain open until the possibility of
//       the line or name getting pulled has ended. The file gets closed
//       when a new character is requested, not when the file actually closes.
//
// (ENH) Buffer some amount of text and place tokens in some kind of
//       buffer to make it quicker to scan them.
//

static struct file_stack
{
    char *fname;
    FILE *fp;
    int line;
    int index;
    struct file_stack *next;
} *pfile_stack = NULL;
static int isinit = 0;
static int tot_lines = 0;
static int close_file_flag = 0;

static void close_file(void)
{
    //ENTER();
    struct file_stack *tfs;

    if (NULL != pfile_stack)
    {
        INFO("closing file: %s", pfile_stack->fname);
        tfs = pfile_stack;
        pfile_stack = tfs->next;
        fclose(tfs->fp);
        free(tfs->fname);
        free(tfs);
        if (NULL != pfile_stack)
            INFO("switch to file: %s", pfile_stack->fname);
        else
            INFO("no files are open");
    }
    //RET();
}

static void close_all_files(void)
{
    ENTER();
    while (NULL != pfile_stack)
        close_file();
    RET();
}

void init_file_io(void)
{
    ENTER();
    if (!isinit)
    {
        pfile_stack = NULL;
        atexit(close_all_files);
        isinit = 1;
    }
    else
        INFO("multiple calls to file init");

    RET();
}

/*
    All errors are fatal errors.
*/
void open_file(const char *fname)
{
    ENTER();
    struct file_stack *tfs;

    if (NULL == (tfs = calloc(1, sizeof(struct file_stack))))
        FATAL("Cannot allocate memory file new file stack");

    if (NULL == (tfs->fname = strdup(fname)))
        FATAL("Cannot allocate memory file file name: %s", fname);

    if (NULL == (tfs->fp = fopen(fname, "r")))
        FATAL("Cannot open input file: %s: %s", fname, strerror(errno));

    tfs->line = 1;
    tfs->index = 1;
    if (NULL != pfile_stack)
        tfs->next = pfile_stack;
    pfile_stack = tfs;

    INFO("Opened file: %s", fname);
    RET();
}

/*
    Returns every  character found in the stream. If it's the end of the
    file, then return 0x01. If it's the end of the input, then return 0x00.

    Does not support unicodes.
*/
int get_char(void)
{
    int ch;

    if (NULL != pfile_stack)
    {
        // this causes the file to be closed after we are finished
        // with it.
        if (close_file_flag)
        {
            close_file();
            close_file_flag = 0;
            if (pfile_stack == NULL)
                return 0x00;
            else
                return get_char();  // recurse to get a character.
        }

        ch = fgetc(pfile_stack->fp);
        if (ch == EOF)
        {
            close_file_flag = 1;
            return 0x01;
        }
        else if (ch == '\n')
        {
            pfile_stack->line++;
            pfile_stack->index = 1;
            tot_lines++;
            return ch;
        }
        else
        {
            pfile_stack->index++;
            return ch;
        }
    }
    return 0x00;
}

void unget_char(int ch)
{
    ENTER();
    if (NULL != pfile_stack)
    {
        ungetc(ch, pfile_stack->fp);
        pfile_stack->index--;
    }
    RET();
}

int line_number(void)
{
    ENTER();
    if (NULL != pfile_stack)
    {
        VRET(pfile_stack->line);
    }
    else
    {
        INFO("no file is open");
        VRET(-1);
    }
}

int line_index(void)
{
    ENTER();
    if (NULL != pfile_stack)
    {
        VRET(pfile_stack->index);
    }
    else
    {
        INFO("no file is open");
        VRET(-1);
    }
}

const char *file_name(void)
{
    ENTER();
    if (NULL != pfile_stack)
    {
        VRET(pfile_stack->fname);
    }
    else
    {
        INFO("no file is open");
        VRET("\"no open file\"");
    }
}

int total_lines(void)
{
    return tot_lines;
}
