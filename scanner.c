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
#include "errors.h"

/*
    Internally, the scanner is just a big ugly state machine
*/
typedef enum
{
    ILLEGAL = -1,
    NUMERIC,   // [0-9\.] Note that the dot depends on the state
    SYMCHARS,  // [_$@?!A-Za-z]
    SQUOTE,    // \'
    DQUOTE,    // \"
    HASH,      // # comment char
    SINGLES,   // [\(\)\{\}\[\]\,\;\:\.]
    WHITESP,   // ' ', '\t', '\n', '\r'
    OPERATORS, // %^&*-+=/!|<>
    END_INPUT,
    END_FILE,
    CHAR_TABLE_SIZE = 256
} character_types_t;
character_types_t char_table[CHAR_TABLE_SIZE];

typedef struct
{
    const char *str;
    token_t tok;
} token_map_t;

// This table must be maintained in sorted order.
static const token_map_t keywords_map[] = {
    {"and", AND_TOK},
    {"break", BREAK_TOK},
    {"case", CASE_TOK},
//    {"class", CLASS_TOK},
    {"cont", CONTINUE_TOK},
    {"continue", CONTINUE_TOK},
    {"create", CREATE_TOK},
    {"destroy", DESTROY_TOK},
    {"else", ELSE_TOK},
    {"eq", EQUAL_TOK},
    {"except", EXCEPT_TOK},
    {"exception", EXCEPT_TOK},
    {"false", FALSE_TOK},
    {"float", FLOAT_DEF_TOK},
    {"for", FOR_TOK},
//    {"func", FUNC_TOK},
//    {"function", FUNC_TOK},
    {"geq", GREATER_OR_EQUAL_TOK},
    {"gt", GREATER_TOK},
    {"if", IF_TOK},
    {"import", IMPORT_TOK},
    {"int", INT_DEF_TOK},
    {"integer", INT_DEF_TOK},
    {"leq", LESS_OR_EQUAL_TOK},
    {"lt", LESS_TOK},
    {"neq", NEQUAL_TOK},
    {"nill", NIL_TOK},
    {"no", FALSE_TOK},
    {"not", NOT_TOK},
    {"nothing", NIL_TOK},
    {"null", NIL_TOK},
    {"or", OR_TOK},
    {"private", PRIVATE_TOK},
    {"protected", PROTECTED_TOK},
    {"public", PUBLIC_TOK},
    {"return", RETURN_TOK},
    {"str", STR_DEF_TOK},
    {"string", STR_DEF_TOK},
    {"switch", SWITCH_TOK},
    {"true", TRUE_TOK},
    {"try", TRY_TOK},
    {"uint", UINT_DEF_TOK},
    {"unsigned", UINT_DEF_TOK},
    {"while", WHILE_TOK},
    {"yes", TRUE_TOK}};
#define MAP_SIZE(m) (sizeof(m) / sizeof(token_map_t))
#define TOKEN_BUFFER_SIZE 1024 * 64
#define PREV_CHAR() token_buffer[(token_buffer_index >= 1) ? token_buffer_index - 1 : 0]
#define CHAR_TYPE(c) char_table[ch]
static char token_buffer[TOKEN_BUFFER_SIZE];
static int token_buffer_index = 0;

static inline void clear_buffer(void)
{
    memset(token_buffer, 0, sizeof(token_buffer));
    token_buffer_index = 0;
}

static inline void add_char(int ch)
{
    token_buffer[token_buffer_index] = ch;
    token_buffer_index++;
    if (token_buffer_index >= TOKEN_BUFFER_SIZE)
    {
        FATAL("token buffer overrun");
        clear_buffer();
    }
}

// returns the token if the str is in the keyword list, otherwise, returns 0.
static token_t
bfind(const token_map_t *toks, int size, const char *str)
{
    int first = 0;
    int last = size - 1;
    int middle = (first + last) / 2;
    int result;

    while (first <= last)
    {
        result = strcmp(str, toks[middle].str);
        if (result > 0)
            first = middle + 1;
        else if (result == 0)
            return toks[middle].tok;
        else
            last = middle - 1;

        middle = (first + last) / 2;
    }
    return 0;
}

static token_t check_keyword(const char *str)
{
    return bfind(keywords_map, MAP_SIZE(keywords_map), str);
}

/*
    Public interface
*/
void init_scanner(const char *fname)
{
    int i;
    char *str;
    
    ENTER();
    // init the file_io
    init_file_io();

    // set up the scanner's internal tables
    for (i = 0; i < CHAR_TABLE_SIZE; i++)
        char_table[i] = ILLEGAL;

    for (i = '0'; i <= '9'; i++)
        char_table[i] = NUMERIC;

    for (i = 'A'; i <= 'Z'; i++)
        char_table[i] = SYMCHARS;

    for (i = 'a'; i <= 'z'; i++)
        char_table[i] = SYMCHARS;

    char_table['_'] = SYMCHARS;
    char_table['$'] = SYMCHARS;
    char_table['\''] = SQUOTE;
    char_table['\"'] = DQUOTE;
    char_table[0x01] = END_FILE;
    char_table[0x00] = END_INPUT;
    char_table['#'] = HASH;

    str = " \n\r\t";
    for (i = 0; str[i] != 0; i++)
        char_table[(int)str[i]] = WHITESP;

    str = "(){}[]:;,.";
    for (i = 0; str[i] != 0; i++)
        char_table[(int)str[i]] = SINGLES;

    str = "%^&*-+=/!|<>";
    for (i = 0; str[i] != 0; i++)
        char_table[(int)str[i]] = OPERATORS;

    if (fname != NULL)
        open_file(fname);
    RET();
}

/*
    When this is entered, a non-alphanumeric character has been scanned
    and placed in the token buffer. It is not known if it is a single
    character operator or a multi-character operator.
*/
static token_t get_operator()
{
    int ch;
    token_t retv = ERROR_TOK;

    switch (token_buffer[0])
    {
        // these are always single characters
    case '*':
        retv = MUL_TOK;
        break;
    case '/':
        retv = DIV_TOK;
        break;
    case '%':
        retv = MOD_TOK;
        break;
    case '~':
        retv = LNOT_TOK;
        break;
    case '^':
        retv = LXOR_TOK;
        break;

        // these may be 1 or 2 chars long
    case '!':
        ch = get_char();
        if (ch == '!')
        {
            add_char(ch);
            retv = NOT_TOK;
        }
        else
        {
            unget_char(ch);
            retv = NOT_TOK;
        }
        break;
    case '|':
        ch = get_char();
        if (ch == '|')
        {
            add_char(ch);
            retv = OR_TOK;
        }
        else
        {
            unget_char(ch);
            retv = LOR_TOK;
        }
        break;
    case '&':
        ch = get_char();
        if (ch == '&')
        {
            add_char(ch);
            retv = AND_TOK;
        }
        else
        {
            unget_char(ch);
            retv = LAND_TOK;
        }
        break;
    case '>':
        ch = get_char();
        if (ch == '>')
        {
            add_char(ch);
            retv = GREATER_TOK;
        }
        else if (ch == '=')
        {
            add_char(ch);
            retv = GREATER_OR_EQUAL_TOK;
        }
        else
        {
            unget_char(ch);
            retv = LSHR_TOK;
        }
        break;
    case '<':
        ch = get_char();
        if (ch == '<')
        {
            add_char(ch);
            retv = LESS_TOK;
        }
        else if (ch == '=')
        {
            add_char(ch);
            retv = LESS_OR_EQUAL_TOK;
        }
        else
        {
            unget_char(ch);
            retv = LSHL_TOK;
        }
        break;
    case '+':
        ch = get_char();
        if (ch == '+')
        {
            add_char(ch);
            retv = INCREMENT_TOK;
        }
        else
        {
            unget_char(ch);
            retv = PLUS_TOK;
        }
        break;
    case '-':
        ch = get_char();
        if (ch == '-')
        {
            add_char(ch);
            retv = DECREMENT_TOK;
        }
        else
        {
            unget_char(ch);
            retv = MINUS_TOK;
        }
        break;
    case '=':
        ch = get_char();
        if (ch == '=')
        {
            add_char(ch);
            retv = EQUAL_TOK;
        }
        else
        {
            unget_char(ch);
            retv = ASSIGN_TOK;
        }
        break;
    default:
        INTERNAL("invalid state found in %s", __func__);
    }
    return retv;
}

/*
    When this is entered, a single character token has been scanned and
    placed in the token buffer. This function simply finds out which one
    and retuns the value.
*/
static token_t get_single()
{
    token_t retv = ERROR_TOK;

    switch (token_buffer[0])
    {
    case '(':
        retv = OPAREN_TOK;
        break;
    case ')':
        retv = CPAREN_TOK;
        break;
    case '{':
        retv = OCURLY_TOK;
        break;
    case '}':
        retv = CCURLY_TOK;
        break;
    case '[':
        retv = OSQU_TOK;
        break;
    case ']':
        retv = CSQU_TOK;
        break;
    case ';':
        retv = SEMI_TOK;
        break;
    case ':':
        retv = COLON_TOK;
        break;
    case ',':
        retv = COMMA_TOK;
        break;
    case '.':
        retv = DOT_TOK;
        break;
    default:
        INTERNAL("invalid state found in %s", __func__);
    }
    return retv;
}

// Read hex digits and put the resulting 2 digit numbers into the string. If
// the number of hex digits found is odd, then the odd one will be the high
// order nibble in the word. Any number of digits can be included.
static void do_hex_escape(void)
{
    int ch;
    int finished = 0;
    int count = 0;
    char buf[4];

    buf[0] = '0';
    buf[1] = '0';
    buf[2] = 0;
    while (!finished)
    {
        ch = get_char();
        if (ch == END_FILE || ch == END_INPUT)
        {
            warning("unexpected end of file encountered reading string");
            finished++;
        }
        else
        {
            if (isxdigit(ch))
            {
                buf[count++] = ch;
                if (count >= 2)
                {
                    add_char((int)strtol(buf, NULL, 16));
                    buf[0] = '0';
                    buf[1] = '0';
                    buf[2] = 0;
                    count = 0;
                }
            }
            else
            {
                add_char(ch);
                finished++;
            }
        }
    }
}

static void do_escape(void)
{
    int ch;
    int finished = 0;

    ch = get_char();
    if (ch == END_FILE || ch == END_INPUT)
    {
        warning("unexpected end of file encountered reading string");
        finished++;
    }
    else
    {
        if (ch == 'x' || ch == 'X')
            do_hex_escape();
        else
        {
            switch (ch)
            {
            case 'a':
                add_char('\a');
                break;
            case 'b':
                add_char('\b');
                break;
            case 'e':
                add_char(0x1b);
                break;
            case 'f':
                add_char('\f');
                break;
            case 'n':
                add_char('\n');
                break;
            case 'r':
                add_char('\r');
                break;
            case 't':
                add_char('\t');
                break;
            case 'v':
                add_char('\v');
                break;
            default:
                add_char(ch);
            }
        }
    }
}

/*
    When this is entered, a double quote character has been read but not
    placed in the token buffer. It is not know if the string is a multi-line
    string or a simple string. Multi-line strings are introduced and ended
    with a "" token. Double quoted strings are copied into the token buffer
    and escapes are interpreted as well as hex numbers with the form of
    \xNNNN. The escapes that are interpreted are exactly the same as any
    recent version of ANSI C.
*/
static token_t get_dquote()
{
    int ch;
    int finished = 0;
    token_t retv = ERROR_TOK;
    int state = 0;

    while (!finished)
    {
        ch = get_char();
        if (ch == END_FILE || ch == END_INPUT)
        {
            warning("unexpected end of file encountered reading string");
            finished++;
        }
        else
        {
            switch (state)
            {
            case 0: // check to see if we have a multi-line
                if (ch != '\"')
                {
                    state = 1;
                    add_char(ch);
                }
                else
                    state = 2;
                break;
            case 1: // single line cannot have a '\n'
                if (ch == '\n')
                {
                    syntax("unterminated quoted string");
                    finished++;
                }
                else if (ch == '\"')
                {
                    retv = STRING_TOK;
                    finished++;
                }
                else if (ch == '\\')
                    do_escape();
                else
                    add_char(ch);
                break;
            case 2:             // copy a multi-line string
                if (ch == '\"') // could be the end of the string
                    state = 3;
                else if (ch == '\\')
                    do_escape();
                else
                    add_char(ch);
                break;
            case 3:
                if (ch == '\"') // found the end of the string
                {
                    retv = STRING_TOK;
                    finished++;
                }
                else if (ch == '\\')
                {
                    add_char('\"'); // else save the quote
                    do_escape();
                }
                else
                {
                    add_char('\"'); // else save the quote
                    add_char(ch);   // and the new character
                    state = 2;      // go back to reading a multi-line string
                }
                break;
            default:
                INTERNAL("unexpected state in %s", __func__);
                break;
            }
        }
    }
    return retv;
}

/*
    When this is entered, a single quote character has been read, but not
    placed in the token buffer. It is not know if the string is a multi-line
    string or a simple string. Multi-line strings are introduced and ended
    with a '' token. Single quoted strings are copied into the token buffer
    without any modifications and no checking is done on what is read from
    the file.
*/
static token_t get_squote()
{
    int ch;
    int finished = 0;
    token_t retv = ERROR_TOK;
    int state = 0;

    while (!finished)
    {
        ch = get_char();
        if (ch == END_FILE || ch == END_INPUT)
        {
            warning("unexpected end of file encountered reading string");
            finished++;
        }
        else
        {
            switch (state)
            {
            case 0: // check to see if we have a milti-line
                if (ch != '\'')
                {
                    state = 1;
                    add_char(ch);
                }
                else
                    state = 2;
                break;
            case 1: // single line cannot have a '\n'
                if (ch == '\n')
                {
                    syntax("unterminated quoted string");
                    finished++;
                }
                else if (ch == '\'')
                {
                    retv = STRING_TOK;
                    finished++;
                }
                else
                    add_char(ch);
                break;
            case 2:             // copy a multi-line string
                if (ch == '\'') // could be the end of the string
                    state = 3;
                else
                    add_char(ch);
                break;
            case 3:
                if (ch == '\'') // found the end of the string
                {
                    retv = STRING_TOK;
                    finished++;
                }
                else
                {
                    add_char('\''); // else save the single quote
                    add_char(ch);   // and the new character
                    state = 2;      // go back to reading a multi-line string
                }
                break;
            default:
                INTERNAL("unexpected state in %s", __func__);
                break;
            }
        }
    }
    return retv;
}

/*
    When this is entered, a valid alphabetic character has been placed into
    the token buffer. It is unknown if the thing being read is a keyword or
    an actual symbol. That is determined at the end of the read.
*/
static token_t get_symbol()
{
    int ch;
    int finished = 0;
    token_t retv = ERROR_TOK;

    while (!finished)
    {
        ch = get_char();
        if (ch == END_FILE || ch == END_INPUT)
        {
            warning("unexpected end of file encountered reading symbol");
            finished++;
        }
        else
        {
            if (CHAR_TYPE(ch) == SYMCHARS || CHAR_TYPE(ch) == NUMERIC)
                add_char(ch);
            else
            {
                unget_char(ch);
                finished++;
            }
        }
    }

    retv = check_keyword(token_buffer);
    if (retv == 0)
        retv = SYMBOL_TOK;
    return retv;
}

/*
    When this is entered, there is "0x" in the token buffer.
*/
static token_t get_hex(void)
{
    int ch;
    int finished = 0;
    token_t retv = ERROR_TOK;

    while (!finished)
    {
        ch = get_char();
        if (ch == END_FILE || ch == END_INPUT)
        {
            warning("unexpected end of file encountered reading hex number");
            finished++;
        }
        else if (isxdigit(ch))
            add_char(ch);
        else
        {
            unget_char(ch);
            finished++;
            retv = UINT_TOK;
        }
    }
    return retv;
}

/*
    When this is entered, there are digits and a '.' in the token buffer.
*/
static token_t get_float(void)
{
    int ch;
    int finished = 0;
    int state = 0;
    token_t retv = ERROR_TOK;

    while (!finished)
    {
        ch = get_char();
        if (ch == END_FILE || ch == END_INPUT)
        {
            warning("unexpected end of file encountered reading float");
            finished++;
        }
        else
        {
            switch (state)
            {
            case 0:
                if (isdigit(ch))
                    add_char(ch);
                else if (ch == 'e' || ch == 'E')
                {
                    add_char(ch);
                    state = 1; // check for a sign
                }
                else
                {
                    unget_char(ch);
                    finished++;
                    retv = FLOAT_TOK;
                }
                break;
            case 1:
                if (ch == '-' || ch == '+')
                {
                    add_char(ch);
                    state = 2;
                }
                else if (isdigit(ch))
                {
                    add_char(ch);
                    state = 2;
                }
                else
                {
                    syntax("malformed floating point number: %s", token_buffer);
                    unget_char(ch);
                    unget_char(PREV_CHAR());
                    finished++;
                    // retv is ERROR_TOK
                }
                break;
            case 2: // read digits until the end
                if (isdigit(ch))
                    add_char(ch);
                else if (CHAR_TYPE(ch) != OPERATORS && CHAR_TYPE(ch) != SINGLES && CHAR_TYPE(ch) != WHITESP)
                {
                    syntax("malformed floating point number: %s", token_buffer);
                    unget_char(ch);
                    finished++;
                    // retv is ERROR_TOK
                }
                else
                {
                    unget_char(ch);
                    finished++;
                    retv = FLOAT_TOK;
                }
                break;
            }
        }
    }
    return retv;
}

/*
    Handle reading a number. When this is entered, a numeral has been added
    to the token buffer. It is unknown if the number is an int, a float, or
    an unsigned. An int is assumed until proven otherwise.
*/
static token_t get_number(void)
{
    int ch;
    int finished = 0;
    int state = 0;
    token_t retv = ERROR_TOK;

    while (!finished)
    {
        ch = get_char();
        if (ch == END_FILE || ch == END_INPUT)
        {
            warning("unexpected end of file encountered reading number");
            finished++;
        }
        else
        {
            switch (state)
            {
            case 0:
                if (PREV_CHAR() == '0')
                { // then this must be a hex number or a float. Octals are not supported.
                    if (ch == 'x' || ch == 'X')
                    {
                        add_char(ch);
                        retv = get_hex();
                        finished++;
                    }
                    else if (ch == '.')
                    {
                        add_char(ch);
                        retv = get_float();
                        finished++;
                    }
                    else if (isdigit(ch))
                    { // issue a warning and continue. Leading '0' are ignored.
                        warning("leading zeros are ignored. Octals are not supported.");
                        add_char(ch);
                    }
                }
                else if (isdigit(ch))
                {
                    add_char(ch);
                    state = 1; // reading an integer.
                }
                else if (ch == '.')
                { // single digit infornt of a '.'
                    add_char(ch);
                    retv = get_float();
                    finished++;
                }
                else
                {
                    unget_char(ch); // have a single digit number
                    finished++;
                    retv = INT_TOK;
                }
                break;
            case 1: // reading an int
                if (isdigit(ch))
                    add_char(ch);
                else if (ch == '.')
                { // single digit infornt of a '.'
                    add_char(ch);
                    retv = get_float();
                    finished++;
                }
                else
                {
                    unget_char(ch); // have a single digit number
                    finished++;
                    retv = INT_TOK;
                }
                break;
            }
        }
    }
    return retv;
}

/*
    Discard comments. THere are 3 kinds of comments. String, line, and block.
    string comment:
        not in a comment # inside the comment # not in a comment.
        Another "#" was seen that is not inside a string and before a new line.
    line comment:
        not in the comment # inside the comment
        A new line was seen before another "#" was seen.
    block comment:
        ## ... ##
        Everything between the "##" tokens is in the comment. More than 2
        consecutive "#" are allowed and a single consecutive "#" inside the
        comment does not end it. For example the following is a valid comment.
        #####
        # inside the comment
        #####

    When this is entered, the token_buffer is empty and a "#" has been seen.
*/
static void get_comment(void)
{
    int ch;
    int finished = 0;
    int state = 0;

    while (!finished)
    {
        ch = get_char();
        if (ch == END_FILE || ch == END_INPUT)
        {
            warning("unexpected end of file encountered reading comment");
            finished++;
        }
        else
        {
            switch (state)
            {
            case 0:
                if (ch == '#')
                    state = 1; // this is a multi-line comment
                else
                    state = 2; // not a multi line comment
                break;
            case 1: // reading a multi-line comment, eat "#" until one is not found
                if (ch != '#')
                    state = 3; // could be the end of the multi-line comment
                // else simply ignore the "#"
                break;
            case 2: // reading a not multi line comment
                if (ch == '#' || ch == '\n')
                    finished++; // comment ends
                // else ignore the character
                break;
            case 3: // could be the end of a multi-line comment, or not
                if (ch == '#')
                    state = 4;
                // else ignore the character
                break;
            case 4:
                if (ch == '#')
                    state = 5; // eat "#" until the end
                else
                    state = 3; // go back to eating characters
                break;
            case 5:
                if (ch != '#')
                {
                    unget_char(ch);
                    finished++;
                }
                break;
            default:
                INTERNAL("invalid state encountered while reading comment");
            }
        }
    }
}

/*
    Main entry point to this module. Call this to scan and return a token.
*/
token_t get_token(void)
{
    ENTER();
    int ch;
    int finished = 0;
    token_t retv;

    clear_buffer();
    while (!finished)
    {
        ch = get_char();
        switch (CHAR_TYPE(ch))
        {
        case ILLEGAL:
            ERROR("illegal character encountered: 0x%02X. Discarded.", ch);
            break;
        case NUMERIC:
            add_char(ch);
            retv = get_number();
            finished++;
            break;
        case SYMCHARS:
            add_char(ch);
            retv = get_symbol();
            finished++;
            break;
        case SQUOTE:
            retv = get_squote();
            finished++;
            break;
        case DQUOTE:
            retv = get_dquote();
            finished++;
            break;
        case HASH:
            get_comment();
            break;
        case SINGLES:
            add_char(ch);
            retv = get_single();
            finished++;
            break;
        case WHITESP:
            /* do nothing, skip white space */
            break;
        case OPERATORS:
            add_char(ch);
            retv = get_operator();
            finished++;
            break;
        case END_INPUT:
            retv = END_OF_INPUT;
            finished++;
            break;
        case END_FILE:
            retv = END_OF_FILE;
            finished++;
            break;
        default:
            INTERNAL("scanner invalid state");
            break;
        }
    }
    DEBUG(3, "returning token: %s (%d)", token_buffer, retv);
    VRET(retv);
}

const char *get_token_string(void)
{
    return token_buffer;
}

#if _TESTING

int main(void)
{
    token_t tok;

    init_logging(LOG_STDERR);
    set_debug_level(10);
    init_scanner("tests/scanner1.txt");

    while (tok != END_OF_INPUT)
    {
        tok = get_token();
        printf("token: %d str: \"%s\" desc: %s\n", tok, get_token_string(), token_to_msg(tok));
        printf("   line: %d index: %d\n", line_number(), line_index());
    }
    return 0;
}

#endif