PROG   := server
CC     := gcc
CFLAGS := -g -O2 -Wall -Wextra -Wwrite-strings -Wformat=2 -Wconversion -Wmissing-declarations -Wmissing-prototypes
LDFLAGS:= -levent -lconfig -ldl
CCDYNAMICFLAGS := ${CFLAGS} -fPIC
LDDYNAMICFLAGS := -shared

SRC := $(wildcard *.c)
OBJ := $(patsubst %.c,%.o,$(SRC))
DEP := $(patsubst %.c,%.deps,$(SRC))

all: $(PROG)

-include $(DEP)

%.deps: %.c
	$(CC) -MM $< >$@

%.o: %.c
	$(CC) $(CCDYNAMICFLAGS) -c -MMD $< -o $@

%.so: %.o
	${CC} ${LDDYNAMICFLAGS} -o $@ $< -o $@

clean:
	rm -f *.o *.d *.deps $(PROG) modules/*.o modules/*.d modules/*.deps modules/*.so

$(PROG): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

