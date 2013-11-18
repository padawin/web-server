PROG   := server
CC     := gcc
CFLAGS := -g -O2 -Wall -Wextra -Wwrite-strings -Wformat=2 -Wconversion -Wmissing-declarations -Wmissing-prototypes
LDFLAGS:= -levent -lconfig -ldl -lmap
CCDYNAMICFLAGS := ${CFLAGS} ${LDFLAGS} -fPIC
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
	rm $(PROG)
	find . -name '*.o' -delete -o -name '*.d' -delete -o -name '*.deps' -delete  -o -name '*.so' -delete

$(PROG): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

