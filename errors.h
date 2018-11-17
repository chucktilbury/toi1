#ifndef _SYNTAX_H_
#define _SYNTAX_H_

#include "scanner.h"
void warning(const char *fmt, ...);
void syntax(const char *fmt, ...);
void expect_token(token_t expect, token_t got, const char* msg);
const char* token_to_str(token_t tok);
const char *token_to_msg(token_t tok);

#endif /* _SYNTAX_H_ */