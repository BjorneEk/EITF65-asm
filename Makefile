
TARGET=EITF65-asm

CC=gcc


INCLUDE_DIR:=src
CFLAGS=-I$(INCLUDE_DIR)
LIBS:=
BIN:=~/Bin
C_SOURCES:=$(wildcard src/*.c *.c )
DEPS:=$(wildcard $(INCLUDE_DIR)/*.h *.h)

OBJ:=${C_SOURCES:.c=.o}

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

run: $(TARGET)
	./$(TARGET)

install: $(TARGET)
	cp $(TARGET) $(BIN)
clean:
	$(RM) src/*.bin src/*.o src/*.dis src/*.elf
	$(RM) lib/*.o
	$(RM) $(OBJ)
	$(RM) $(TARGET)
