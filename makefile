CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -g
LDFLAGS := -I.

SRCFILES := $(wildcard *.c)
OBJFILES := $(patsubst %.c,%.o,$(SRCFILES))

main: $(OBJFILES)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

clean:
	rm -rf *.o main
