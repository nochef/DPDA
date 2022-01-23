CC= gcc
CFLAGS= -g -W -Wall -Werror -std=c99 -I$(testlib) -I.
OBJ = main.o ./lib/cJSON.o
DEPS = ./lib/cJson.h

%.o: %.c $(DEPS)
		$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
		$(CC) -o $@ $^ $(CFLAGS)
