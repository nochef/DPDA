#include <stdio.h>
#include <stdlib.h>

#define STACK_CAP 1024
#define STATE_CAP 64

typedef struct _DPDA {
	int stack[STACK_CAP];
	size_t stackHead;
	
	int transitions[STATE_CAP][STATE_CAP];

	char* input;
	size_t inputHead;
} DPDA;

int main(int argc, char **argv) {
	DPDA dpda;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			dpda.transitions[i][j] = i + j;
		}
	}

	printf("%d\n", dpda.stack[0]);
	printf("%d\n", dpda.stack[1]);

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			printf("%d,", dpda.transitions[i][j]);
		}
		printf("\n");
	}

}
