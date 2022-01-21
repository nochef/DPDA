#include <stdio.h>

#define STACK_CAP 1024

typedef struct _Stack {
	int values[STACK_CAP];
} Stack;

typedef struct _DPDA {
	Stack stack;
} DPDA;


int main(int argc, char **argv) {
	DPDA dpda;
	for (int i = 0; i < STACK_CAP; i++) {
		dpda.stack.values[i] = i;
	}
	for (int i = 0; i < STACK_CAP; i++) {
		printf("%d,", dpda.stack.values[i]);
	}
}
