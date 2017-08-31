CC = gcc
CLFAGS = -std=c99 -Wall -Wextra -pedantic
APP = gif2bmp
OBJS = $(subst .c,.o,$(wildcard *.c))

.PHONY: clean debug

all: $(APP)
debug: CFLAGS += -g
debug: $(APP)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(APP): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf *.o $(APP) *.log
