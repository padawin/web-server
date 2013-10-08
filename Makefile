PROG   := server
CC     := gcc
CFLAGS := -g -O2 -Wall -Wextra -Wwrite-strings -Wformat=2 -Wconversion -Wmissing-declarations -Wmissing-prototypes
LDFLAGS:= -levent

SRC := $(wildcard *.c)
OBJ := $(patsubst %.c,%.o,$(SRC))
DEP := $(patsubst %.c,%.deps,$(SRC))

all: $(PROG)

-include $(DEP)

%.deps: %.c
	$(CC) -MM $< >$@

%.o: %.c
	$(CC) $(CFLAGS) -c -MMD $< -o $@

clean:
	rm -f *.o *.d *.deps $(PROG)

$(PROG): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

