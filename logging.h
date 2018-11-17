#ifndef _LOGGING_H_
#define _LOGGING_H_

typedef enum typelog_t
{
    DEBUG,
    ENTER,
    RETURN,
    INFO,
    WARN,
    ERROR,
    SYNTAX,
    INTERNAL,
    FATAL,
} typelog_t;

typedef enum logging_flags_t
{
    LOG_STDOUT = 0x01,
    LOG_STDERR = 0x02,
    LOG_FILE = 0x04,
    LOG_APPEND = 0x08,
} logging_flags_t;

void show_msg(typelog_t type, int lev, const char *func, int line, const char *fmt, ...);

#ifdef _DEBUGGING
#define DEBUG(lev, fmt, ...) show_msg(DEBUG, (lev), __func__, __LINE__, fmt, ##__VA_ARGS__)
#define ENTER() show_msg(ENTER, 5, __func__, __LINE__, "")
#define VRET(v) do{show_msg(RETURN, 5, __func__, __LINE__, ##v);return((v));}while(0)
#define RET() do{show_msg(RETURN, 5, __func__, __LINE__, "");return;}while(0)
#else
#define DEBUG(lev, fmt, ...)
#define ENTER()
#define VRET(v) return(v)
#define RET() return
#endif

#define INFO(fmt, ...) show_msg(INFO, 1, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) show_msg(ERROR, 0, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define INTERNAL(fmt, ...) show_msg(INTERNAL, 0, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define FATAL(fmt, ...) show_msg(FATAL, 0, __func__, __LINE__, fmt, ##__VA_ARGS__)

#endif /* _LOGGING_H_ */