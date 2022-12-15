
TARGET=EIT65-asm

CC=gcc


INCLUDE_DIR:=src
CFLAGS=-I$(INCLUDE_DIR) -F /Library/Frameworks
LIBS:=-framework UL

C_SOURCES:=$(wildcard src/*.c *.c )
DEPS:=$(wildcard $(INCLUDE_DIR)/*.h *.h)

OBJ:=${C_SOURCES:.c=.o}

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

run: $(TARGET)
	./$(TARGET)


clean:
	$(RM) src/*.bin src/*.o src/*.dis src/*.elf
	$(RM) lib/*.o
	$(RM) $(OBJ)
