CC = gcc
CLFAGS = -std=c99 -Wall -Wextra -pedantic
APP = gif2bmp
OBJS = $(subst .c,.o,$(wildcard *.c))

# This is for testing purposes.
OBJS_NO_MAIN = $(filter-out main.o, $(OBJS))
TEST_FILES = $(wildcard ./tests/*Test.c)

.PHONY: clean debug

all: $(APP)
debug: CFLAGS += -g
debug: $(APP)

test: $(OBJS_NO_MAIN)
	rm -f testResult
	$(CC) $(CFLAGS) $(TEST_FILES) $^ -o testResult
	./testResult
	rm testResult

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(APP): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf *.o $(APP) *.log
