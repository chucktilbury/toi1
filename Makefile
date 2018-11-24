
OBJS 	= 	file_io.o \
			logging.o \
			errors.o \
			scanner.o \
			context.o \
			hash_table.o \
			symbols.o \
			xxhash.o \
			toi.o

HEADERS	=	file_io.h \
			logging.h \
			errors.h \
			scanner.h \
			context.h \
			hash_table.h \
			symbols.h \
			xxhash.h \
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
toi.o: toi.c $(HEADERS)

clean:
	rm -f $(TARGET) $(OBJS)