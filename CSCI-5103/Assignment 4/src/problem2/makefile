SRCS = $(wildcard *.c)

PROGS = $(patsubst %.c,%,$(SRCS))

all: $(PROGS)

%: %.c
	$(CC) $(CFLAGS)  -o $@ $< -lpthread

clean:
	rm -f $(PROGS)
	rm producer_black.txt
	rm producer_red.txt
	rm producer_white.txt
	rm consumer.txt
