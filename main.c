#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>

#include "lib/cJSON.h"

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

	printf("    Transitions:\n");
	size_t i = 0;
	Transition t;
	while(dpda->transitions[i].valid != 0) {
		t = dpda->transitions[i];
		char *to_stack = strlen(t.to_stack) == 0 ? "_" : t.to_stack;
		char from_input = t.from_input == NONEC ? '_' : t.from_input;
		char from_stack = t.from_stack == NONEC ? '_' : t.from_stack;
		printf("        from (%d, %c, %c) to (%d, %s)\n", t.from_state, from_input, from_stack, t.to_state, to_stack);
		i++;
	}

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

int loop(DPDA *dpda) {
	Transition *t = findTransition(dpda);

	// loop until no transition found
	while (t != NULL) {
		
#if DEBUG
		dumpDPDA(dpda);
#endif
		// Change stack
		memcpy(&dpda->stack[dpda->stackSize - 1], t->to_stack, strlen(t->to_stack));
		dpda->stackSize += strlen(t->to_stack) - 1;
		if (dpda->stackSize >= STACK_CAP) dpda->stackSize = 0;

		// Change state
		dpda->state = t->to_state;

		// Move input
		if (dpda->inputHead < strlen(dpda->input))
			dpda->inputHead++;
		
		t = findTransition(dpda);
	}

	int result = check_DPDA_accepts(dpda);
	dumpDPDA(dpda);
	return result == 1 ? 0 : -1;
}

#define JSON_INPUT_SYMBOLS "input_symbols"
#define JSON_STACK_SYMBOLS "stack_symbols"
#define JSON_STACK_INIT "stack_init"
#define JSON_STATES "states"
#define JSON_ACCEPTING "accepting"
#define JSON_TRANSITIONS "transitions"
#define JSON_FROM_STATE "from_state"
#define JSON_FROM_INPUT "from_input"
#define JSON_FROM_STACK "from_stack"
#define JSON_TO_STATE "to_state"
#define JSON_TO_STACK "to_stack"

DPDA* parseDPDA(DPDA *dpda, const char* const path) {
	FILE *f = fopen(path, "r");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *buf = malloc(fsize + 1);
	buf[fsize] = 0;
	fread(buf, fsize, 1, f);
	fclose(f);
	
	cJSON *dpda_json = cJSON_Parse(buf);
	if (dpda_json == NULL) {
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL){
			fprintf(stderr, "Error before: %s\n", error_ptr);
		}
		fprintf(stderr, "ERROR: parsing json input failed.\n");
		exit(1);
	}

	cJSON *input_symbols, *stack_symbols, *stack_init, *states, *transitions, *transition, *accepting;

	input_symbols = cJSON_GetObjectItem(dpda_json, JSON_INPUT_SYMBOLS);
	assert(cJSON_IsString(input_symbols));

	stack_symbols = cJSON_GetObjectItem(dpda_json, JSON_STACK_SYMBOLS);
	assert(cJSON_IsString(stack_symbols));

	states = cJSON_GetObjectItem(dpda_json, JSON_STATES);
	assert(cJSON_IsString(states));
	/* char *states_str = cJSON_GetStringValue(states); */
	/* dpda->state = states_str[0] - '0'; */
	
	accepting = cJSON_GetObjectItem(dpda_json, JSON_ACCEPTING);
	assert(cJSON_IsString(accepting));
	
	stack_init = cJSON_GetObjectItem(dpda_json, JSON_STACK_INIT);
	assert(cJSON_IsString(stack_init));
	char *stack_init_str = cJSON_GetStringValue(stack_init);
	memcpy(dpda->stack, stack_init_str, strlen(stack_init_str));
	dpda->stackSize = strlen(stack_init_str);
	
	transitions = cJSON_GetObjectItem(dpda_json, JSON_TRANSITIONS);
	assert(cJSON_IsArray(transitions));

	// TODO: save other information into dpda.
	
	char* acceptingStr = cJSON_GetStringValue(accepting);
	for (size_t i = 0; i < strlen(acceptingStr); i++) {
		assert(acceptingStr[i] >= '0' && acceptingStr[i] <= '9');
		dpda->acceptingStates[acceptingStr[i] - '0'] = 1;
	}
	
	size_t i = 0;
	cJSON_ArrayForEach(transition, transitions){
		int from_state = cJSON_GetNumberValue(cJSON_GetObjectItem(transition, JSON_FROM_STATE));
		char from_input = cJSON_GetStringValue(cJSON_GetObjectItem(transition, JSON_FROM_INPUT))[0];
		char from_stack = cJSON_GetStringValue(cJSON_GetObjectItem(transition, JSON_FROM_STACK))[0];
		int to_state = cJSON_GetNumberValue(cJSON_GetObjectItem(transition, JSON_TO_STATE));
		
		char* to_stack = cJSON_GetStringValue(cJSON_GetObjectItem(transition, JSON_TO_STACK));
		char* dyn_to_stack = malloc(strlen(to_stack) + 1);
		memcpy(dyn_to_stack, to_stack, strlen(to_stack));
		dyn_to_stack[strlen(to_stack)] = 0;
		if (strlen(to_stack) == 0) to_stack = NONES;
		
		dpda->transitions[i] = (Transition) {
			.from_state = from_state,
			.from_input = from_input,
			.from_stack = from_stack,
			.to_state = to_state,
			.to_stack = dyn_to_stack,
			.valid = 1
		};
		
		i++;
	}
	
	cJSON_Delete(dpda_json);
	free(buf);
	return dpda;
}

void deleteDPDA(DPDA *dpda) {
	free(dpda);
}

char* readArgv(char ***argv) {
	char *arg = **argv;
	(*argv)++;
	return arg;
}

int main(int argc, char **argv) {
	(void) argc;
	DPDA *dpda = calloc(1, sizeof(DPDA));
	
	readArgv(&argv);
	char *dpda_config = readArgv(&argv);
	char *input = readArgv(&argv);
	dpda->input = input;
	dpda->inputHead = 0;
	parseDPDA(dpda, dpda_config);
	dumpDPDA(dpda);
	
	int result = loop(dpda);
	
	deleteDPDA(dpda);
	
	return result;
}
