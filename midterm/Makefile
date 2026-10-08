CC      = cc
CFLAGS  = -std=c11 -Wall -Wextra -Wpedantic
LDFLAGS =
TARGET  = ls
OBJS    = ls.o print.o cmp.o

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

ls.o: ls.c ls.h
	$(CC) $(CFLAGS) -c ls.c

print.o: print.c ls.h
	$(CC) $(CFLAGS) -c print.c

cmp.o: cmp.c ls.h
	$(CC) $(CFLAGS) -c cmp.c

test: $(TARGET)
	@echo "== basic =="
	./ls
	@echo "== long =="
	./ls -l ls.c
	@echo "== hidden =="
	./ls -a
	@echo "== inode =="
	./ls -i ls.c
	@echo "== recursive =="
	./ls -R . >/dev/null
	@echo "tests completed"

clean:
	rm -f $(TARGET) $(OBJS)
