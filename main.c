#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#define STACK_CAP 64
#define STATE_CAP 64
#define TRANSITIONS_CAP 1024
#define NONEC '\0'
#define NONES "\0"

#define DEBUG 0

typedef struct _Transition {
	int from_state;
	char from_input;
	char from_stack;

	int to_state;
	char *to_stack;

	int valid;
} Transition;

/*
 * A DPDA consits of 7 elements. They are:
 * 1. States: These are represented by integer values
 * 2. Set of input symbols: These are allowed symbols in the input
 * 3. Set of stack symbols: These are allowed symbols on the stack
 * 4. Start state: The initial state
 * 5. Starting stack symbol: This is the initial symbol on the stack
 * 6. Set of accepting states: trivial
 * 7. Transition function: Transitions are represented by a function that maps
 *        (from_state, from_input, from_stack) to (to_state, to_stack)
 *        where from_input and from_stack are characters and to_stack is a string.
 */
typedef struct _DPDA {
	char* inputSymbols;
	char* input;
	size_t inputHead;
	
	char* stackSymbols;
	char stack[STACK_CAP];
	size_t stackSize;

	uint8_t state;
	uint8_t acceptingStates[STATE_CAP];

	Transition transitions[TRANSITIONS_CAP];
} DPDA;

void dumpStack(DPDA *dpda) {
	if (dpda->stackSize == 0) printf("_");
	for (size_t i = 0; i < dpda->stackSize; i++) {
		printf("%c", dpda->stack[i]);
	}
	printf("\n");
}

void dumpInput(DPDA *dpda) {
	size_t i = 0;
	for (; i < dpda->inputHead; i++) {
		if (dpda->input[i] != NONEC) printf("%c", dpda->input[i]);
		else printf("_");
	}
	printf("->");
	if (i < strlen(dpda->input))
		printf("%s", (dpda->input + i));
	else
		printf("_");
	printf("\n");
}

void dumpDPDA(DPDA *dpda) {
	printf("DPDA:\n");

	printf("    State: %d\n", dpda->state);
	
	printf("    Stack (size = %zu): ", dpda->stackSize);
	dumpStack(dpda);

	printf("    Input (head = %zu): ", dpda->inputHead);
	dumpInput(dpda);

	printf("\n");
}

Transition* findTransition(DPDA *dpda) {
	for (size_t i = 0; i < TRANSITIONS_CAP && dpda->transitions[i].valid == 1; i++) {
		Transition t = dpda->transitions[i];
		// state and input correct
		if (t.from_state == dpda->state && t.from_input == dpda->input[dpda->inputHead]) {
			// stack correct
			if ((dpda->stackSize == 0 && t.from_stack == NONEC) || (dpda->stackSize > 0 && t.from_stack == dpda->stack[dpda->stackSize - 1])) {
				// entry is valid
				if (t.valid == 1) {
					return &dpda->transitions[i];
					
				}
			}
		}
	}
	return NULL;
}

int check_DPDA_accepts(DPDA *dpda) {
	// We accept only if input is empty and we either have empty stack or accepting state
	if (dpda->inputHead == strlen(dpda->input)) {
		if (dpda->stackSize == 0 && !dpda->acceptingStates[dpda->state]) {
			printf("DPDA accepts input '%s' by empty stack\n", dpda->input);
			return 1;
		} else if(dpda->acceptingStates[dpda->state] && dpda->stackSize > 0) {
			printf("DPDA accepts input '%s' by accepting state\n", dpda->input);
			return 1;
		} else if (dpda->acceptingStates[dpda->state] && dpda->stackSize == 0) {
			printf("DPDA accepts input '%s' by accepting state AND empty stack\n", dpda->input);
			return 1;
		} else {
			printf("DPDA does not accept input '%s'\n", dpda->input);
			return 0;
		}
	} else {
		printf("DPDA does not accept input '%s'\n", dpda->input);
		return 0;
	}
}


void loop(DPDA *dpda) {
	Transition *t = findTransition(dpda);

	// loop until no transition found
	while (t != NULL) {
		
#if DEBUG
		dumpDPDA(dpda);
#endif
		memcpy(&dpda->stack[dpda->stackSize - 1], t->to_stack, strlen(t->to_stack));
		if (!(dpda->stackSize == 0 && strlen(t->to_stack) == 0))
			dpda->stackSize += strlen(t->to_stack) - 1;

		dpda->state = t->to_state;
		if (dpda->inputHead < strlen(dpda->input))
			dpda->inputHead++;
		
		t = findTransition(dpda);
	}

	check_DPDA_accepts(dpda);
	dumpDPDA(dpda);
}

// TODO: implement a parser that can operate on any given dpda.
void init_dpda_transition(DPDA *dpda) {
	// Transition = { from_state, from_input, from_stack, to_state, to_stack, valid }
	dpda->transitions[0]  = (Transition) {0, '0', 'S', 1, "S0", 1};
	dpda->transitions[1]  = (Transition) {0, '1', 'S', 1, "S1", 1};
	
	dpda->transitions[2]  = (Transition) {1, '0', '0', 1, "00", 1};
	dpda->transitions[3]  = (Transition) {1, '0', '1', 1, "10", 1};
	dpda->transitions[4]  = (Transition) {1, '1', '0', 1, "01", 1};
	dpda->transitions[5]  = (Transition) {1, '1', '1', 1, "11", 1};
	
	dpda->transitions[6]  = (Transition) {1, '#', '0', 2, "0", 1};
	dpda->transitions[7]  = (Transition) {1, '#', '1', 2, "1", 1};
	
	dpda->transitions[8]  = (Transition) {2, '0', '0', 2, NONES, 1};
	dpda->transitions[9]  = (Transition) {2, '1', '1', 2, NONES, 1};
	dpda->transitions[10] = (Transition) {2, NONEC, 'S', 2, NONES, 1};
}

int main(void) {
	DPDA dpda;
	
	dpda.inputSymbols = "01#";
	dpda.input = "111001#100111";

	dpda.stackSymbols = "01#";
	dpda.stack[0] = 'S';
	dpda.stackSize = 1;

	dpda.acceptingStates[2] = 1;

	dpda.state = 0;

	init_dpda_transition(&dpda);
	
	loop(&dpda);
	
	return 0;
}
