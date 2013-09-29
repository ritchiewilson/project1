TARGET = 3600sh

$(TARGET): $(TARGET).c slow.c
	gcc -std=c99 -O0 -g -lm -Wall -pedantic -Werror -Wextra -o $@ $<
	gcc -std=c99 -O0 -g -lm -Wall -pedantic -Werror -Wextra -o slow slow.c

all: $(TARGET)

test: all
	./test

clean:
	rm $(TARGET)
	rm slow
