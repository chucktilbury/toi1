#ifndef _SYNTAX_H_
#define _SYNTAX_H_

#define WARN(fmt, ...) show_msg(WARN, 0, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define SYNTAX(fmt, ...) show_msg(SYNTAX, 0, __func__, __LINE__, fmt, ##__VA_ARGS__)

#endif /* _SYNTAX_H_ */