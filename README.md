Deterministic Pushdown Automaton

# Simple Usage
```
$ make
$ ./main <dpda_config.json> <input>
```

# Use own DPDA configs
You can use your own DPDA config by creating a simple JSON file. This file can have a few membery:

- Name (optional): This gives the DPDA a name that will be shown in the output.
```json
"name": "MyDPDA",
```
- Input Symbols: All allowed input symbols, character by character.
```json
"input_symbols": "ab",
```
- Stack Symbols: All allowed symbols on the stack, character by character.
```json
"stack_symbols": "Sab",
```
- Stack Init: The inital state of the stack.
```json
"stack_init": "S",
```
- States: Those will be named by a single character. Each character in the given string is one state.
```json
"states": "01",
```
- Accepting States: All accepting states listed character by character.
```json
"accepting": "01",
```
- Transitions: An array of single transitions. Transitions are described below.
```json
"transitions": [
    {
      "from_state": 0,
			"from_input": "a",
			"from_stack": "S",
			"to_state": 0,
			"to_stack": "a"
    		},
		{
			"from_state": 0,
			"from_input": "a",
			"from_stack": "a",
			"to_state": 0,
			"to_stack": "aa"
		},
		{
			"from_state": 0,
			"from_input": "b",
			"from_stack": "a",
			"to_state": 1,
			"to_stack": ""
		},
		{
			"from_state": 1,
			"from_input": "b",
			"from_stack": "a",
			"to_state": 1,
			"to_stack": ""
		}
]
```

# Transitions
A transition consits of 5 values:

1. `"from_state"`: The state the DPDA is in.
2. `"from_input"`: The current symbol of the input.
3. `"from_stack"`: The current symbol on top of the stack.
4. `"to_state"`: The state the DPDA will go to during the transition.
5. `"to_stack"`: The symbols the DPDA will push on top of the stack during the transition.

The first three conditions must be met in order for the DPDA to take this transition.
It will perform a transition by consuming the symbol on top of the stack and then move to `to_state` and push the symbols of
`to_stack` onto the stack.
