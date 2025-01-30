#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "toml-c.h"
#include "machine.h"
#include "par_map.h"
void assign_next_state (void* elem_arg_ptr) {
    par_map_elem_arg_t* args = elem_arg_ptr;
    transition_t *transition = args->element;
    machine_t* machine = args->func_arg;
//    printf("assigning next state..\n");
    for (int k = 0; k < machine->num_states; k++){
        printf("trying machine state: %s\n",machine->states[k].name);
         if(strcmp(machine->states[k].name, transition->next_state_name) == 0) {
                transition->next_state = &(machine->states[k]);
         }
    }
}
void process_transitions(void* elem_arg_ptr) {
    par_map_elem_arg_t* input_args = elem_arg_ptr;
    state_t *state = input_args->element;
    machine_t* machine = input_args->func_arg;
    par_map_args_t args;
    args.array = state->transitions;
    args.elem_size = sizeof(transition_t);
    args.start_idx = 0;
    args.num_elements = state->num_transitions;
    args.func = assign_next_state;
    args.func_arg = machine;
//    printf("processing transitions for state %s..\n", state->name);
    par_map(args,2);
}
void process_states(machine_t* machine){
    par_map_args_t args;
    args.array     = &machine->states;
    args.elem_size = sizeof(state_t);
    args.start_idx = 0;
    args.num_elements = machine->num_states;
    args.func = process_transitions;
    args.func_arg = machine;
//    printf("processing machine...\n");
    par_map(args,2);
}



// Helper function to initialize the tape from a string
void initialize_tape(tape_t *tape, const char* symbols) {
    if (!symbols || *symbols == '\0') {
        // Create a default blank cell if tape is empty
        cell_t* cell = malloc(sizeof(cell_t));
        cell->symbol = '_';
        cell->left = cell->right = NULL;
        tape->head = tape->leftmost = tape->rightmost = cell;
        return;
    }

    cell_t* prev = NULL;
    for (const char* c = symbols; *c; c++) {
        cell_t* new_cell = malloc(sizeof(cell_t));
        new_cell->symbol = *c;
        new_cell->left = prev;
        new_cell->right = NULL;

        if (prev) {
            prev->right = new_cell;
        } else {
            tape->leftmost = new_cell;
        }
        prev = new_cell;
    }
    tape->rightmost = prev;
    tape->head = tape->leftmost;
}


cell_t* add_cell_to_tape(tape_t* tape, char direction) {
    if (!tape || (direction != 'L' && direction != 'R')) {
        return NULL;  // Invalid parameters
    }

    // Create new blank cell
    cell_t* new_cell = (cell_t*)malloc(sizeof(cell_t));
    if (!new_cell) {
        return NULL;  // Memory allocation failed
    }
    
    *new_cell = (cell_t){
        .symbol = '_',  // Default blank symbol
        .left = NULL,
        .right = NULL
    };

    if (!tape->leftmost) {  // Empty tape case
        tape->leftmost = tape->rightmost = tape->head = new_cell;
        return new_cell;
    }

    if (direction == 'L') {
        // Add to left end
        new_cell->right = tape->leftmost;
        tape->leftmost->left = new_cell;
        tape->leftmost = new_cell;
        return new_cell;
    } else {
        // Add to right end
        new_cell->left = tape->rightmost;
        tape->rightmost->right = new_cell;
        tape->rightmost = new_cell;
        return new_cell;
    }
}

// Helper function to clean up machine resources
void free_tape(tape_t* tape){
    cell_t* current = tape->leftmost;
    while (current) {
        cell_t* next = current->right;
        free(current);
        current = next;
    }
}
void free_machine(machine_t* machine) {
    free_tape(&(machine->tape));
    free(machine);
}

void print_tape (tape_t tape) {
    cell_t *current = tape.leftmost;
    while (current) {
        putchar(current->symbol);
        current = current->right;
    }
    putchar ('\n');
}


machine_t* create_machine_from_toml(char* inputbuf) {
    char errbuf[255];
    toml_table_t* root = toml_parse(inputbuf, errbuf, sizeof(errbuf));
    
    if (!root) {
        fprintf(stderr, "TOML parsing error: %s\n", errbuf);
        return NULL;
    }

    machine_t* machine = malloc(sizeof(machine_t));
    if (!machine) {
        toml_free(root);
        return NULL;
    }

    // Parse initial tape
    toml_value_t tape_val = toml_table_string(root, "initial_tape");
    if (!tape_val.ok) {
        fprintf(stderr, "Missing 'initial_tape' in TOML\n");
        free_machine(machine);
        toml_free(root);
        return NULL;
    }
    initialize_tape(&machine->tape, tape_val.u.s);
    free(tape_val.u.s);

    // Parse initial state
    toml_value_t state_val = toml_table_string(root, "initial_state");
    if (!state_val.ok || strlen(state_val.u.s) >= STATE_NAME_LEN) {
        fprintf(stderr, "Missing or invalid 'initial_state'\n");
        free_machine(machine);
        toml_free(root);
        return NULL;
    }
    strncpy(machine->initial_state_name, state_val.u.s, STATE_NAME_LEN);
    free(state_val.u.s);

    // Parse states
    int num_keys = toml_table_len(root);
    for (int i = 0; i < num_keys; i++) {
        int keylen;
        const char* key = toml_table_key(root, i, &keylen);
        
        // Skip special keys
        if (strcmp(key, "initial_tape") == 0 || strcmp(key, "initial_state") == 0) {
            continue;
        }

        toml_table_t* state_table = toml_table_table(root, key);
        if (!state_table) continue;

        if (machine->num_states >= MAX_STATES) {
            fprintf(stderr, "Exceeded maximum number of states\n");
            free_machine(machine);
            toml_free(root);
            return NULL;
        }

        state_t* state = &machine->states[machine->num_states++];
        strncpy(state->name, key, STATE_NAME_LEN);
        state->num_transitions = 0;
        if (strcmp(machine->initial_state_name, key) == 0){
            //set the initial state
            machine->current_state = state;
        }

        // Parse transitions
        int num_trans = toml_table_len(state_table);
        for (int j = 0; j < num_trans; j++) {
            const char* trans_key = toml_table_key(state_table, j, &keylen);
            if (strlen(trans_key) != 1) {
                fprintf(stderr, "Invalid transition key in state %s\n", key);
                continue;
            }

            toml_table_t* trans_table = toml_table_table(state_table, trans_key);
            if (!trans_table) continue;

            transition_t trans = {
                .read = trans_key[0],
                .write = '_',
                .direction = 'S',
                .next_state_name = ' ',
                .next_state = NULL,
            };

            // Parse write symbol
            toml_value_t write_val = toml_table_string(trans_table, "write");
            if (write_val.ok && strlen(write_val.u.s) == 1) {
                trans.write = write_val.u.s[0];
                free(write_val.u.s);
            }

            // Parse direction
            toml_value_t dir_val = toml_table_string(trans_table, "direction");
            if (dir_val.ok && strlen(dir_val.u.s) == 1 && 
                (dir_val.u.s[0] == 'L' || dir_val.u.s[0] == 'R' || dir_val.u.s[0] == 'S')) {
                trans.direction = dir_val.u.s[0];
                free(dir_val.u.s);
            }

            // Parse next_state
            toml_value_t next_val = toml_table_string(trans_table, "next_state");
            if (next_val.ok && strlen(next_val.u.s) < STATE_NAME_LEN) {
                strncpy(trans.next_state_name, next_val.u.s, STATE_NAME_LEN);
                free(next_val.u.s);
            }

            if (state->num_transitions < MAX_TRANSITIONS_PER_STATE) {
                state->transitions[state->num_transitions++] = trans;
            }
        }
    }

    //For every transition rule, next_state will point to NULL if the transition's next_state_name doesn't refer to a valid state because all transitions are initialized with a NULL pointer for next_state

    process_states(machine);

    toml_free(root);
    return machine;
}



