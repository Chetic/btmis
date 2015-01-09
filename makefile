.PHONY: clean
CC := gcc
LD := gcc
AR := ar
CCFLAGS := -c -ggdb -O0 -Iinclude -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include
LDFLAGS := -g -Llib
O_FILES := obj/main.o

all: obj bin bin/btmis

bin/btmis: $(O_FILES)
	$(LD) $(LDFLAGS) $^ -o $@ -lcanusb -ldbus-1

obj bin:
	mkdir -p $@

clean:
	rm -rf bin || :
	rm -rf obj || :

obj/%.o: src/%.c
	$(CC) $(CCFLAGS) $^ -o $@
