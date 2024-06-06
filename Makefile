CFLAGS=-O2 -Wall -Werror -Wimplicit-fallthrough
SRCS=$(wildcard src/*.c)
HDRS=$(wildcard src/*.h)
OBJS=$(patsubst src/%.c, obj/%.o, $(SRCS))
CC=clang

rvsim: $(OBJS)
		$(CC) $(CFLAGS) -lm -o $@ $^ $(LDFLAGS)

$(OBJS): obj/%.o: src/%.c $(HDRS)
		@mkdir -p $$(dirname $@)
			$(CC) $(CFLAGS) -c -o $@ $<

clean:
		rm -rf rvsim obj/

.PHONY: clean
