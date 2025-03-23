CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11
LDFLAGS = -lncurses  # Use this for linker flags

SRC = main.c dir.c
OBJ = $(SRC:.c=.o)
DEPS = dir.h util.h

TARGET = tfm

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)  # Linker flags go here

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@  # No LDFLAGS here

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
