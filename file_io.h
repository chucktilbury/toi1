#ifndef _FILE_IO_H_
#define _FILE_IO_H_

void open_file(const char *fname);
int get_char(void);
void unget_char(int ch);
int line_number(void);
int line_index(void);
const char *file_name(void);
int total_lines(void);

#endif /* _FILE_IO_H_ */