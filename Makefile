CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2
LDLIBS = -lm

SRCS = main.c dijkstra.c grafo.c min_heap.c pgm_io.c output.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: detect

detect: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) detect
