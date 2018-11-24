
OBJS 	= 	file_io.o \
			logging.o \
			errors.o \
			scanner.o \
			context.o \
			hash_table.o \
			symbols.o \
			xxhash.o \
			parse.o \
			class_def.o \
			import_def.o \
			toi.o

HEADERS	=	file_io.h \
			logging.h \
			errors.h \
			scanner.h \
			context.h \
			hash_table.h \
			symbols.h \
			xxhash.h \
			parse.h \
			class_def.h \
			import_def.h \
			toi.h

TARGET 	=	toi
CARGS	=	-Wall -Wextra -g -D_DEBUGGING

.c.o: 
	gcc $(CARGS) -c $<

all: $(TARGET)

$(TARGET): $(OBJS) $(HEADERS)
	gcc -g -o $(TARGET) $(OBJS)

file_io.o: file_io.c $(HEADERS)
logging.o: logging.c $(HEADERS)
errors.o: errors.c $(HEADERS)
scanner.o: scanner.c $(HEADERS)
context.o: context.c $(HEADERS)
hash_table.o: hash_table.c $(HEADERS)
symbols.o: symbols.c $(HEADERS)
xxhash.o: xxhash.c $(HEADERS)
parse.o: parse.c $(HEADERS)
toi.o: toi.c $(HEADERS)
class_def.o: class_def.c  $(HEADERS)
import_def.o: import_def.c $(HEADERS)

clean:
	rm -f $(TARGET) $(OBJS)