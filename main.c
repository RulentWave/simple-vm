#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "machine-builder.c"
#include "read_stdin.c"

int main() {
    char *input = read_stdin();
    machine_t *machine = create_machine_from_toml(input);
    if (!machine) {printf("Error building machine\n"); return 1;}
    print_tape(machine->tape);

    // Main Turing machine simulation loop
    while (true) {
        if (!machine->current_state){ printf("current_state points to NULL. halting...\n"); break;}  // Halt if state not found
        if (strcmp(machine->current_state->name, "halt") == 0){ printf("halting...\n"); break;}  // Halt if state name is "halt"

        // Get current symbol under head
        char read = machine->tape.head->symbol;
//        printf("reading symbol %c\n",read);
        transition_t *t = NULL;
        
        // Find matching transition for current symbol
//        printf("machine->num_states: %d\n", machine->num_states);
//        printf("current_state->name: %s\n", machine->current_state->name);
//        printf("current_state->num_transitions %d\n",machine->current_state->num_transitions);
        for (int i = 0; i < machine->current_state->num_transitions; i++) {
            if (machine->current_state->transitions[i].read == read) {
                t = &(machine->current_state->transitions[i]);
                //printf("Found matching transition for symbol!\n");
                break;
            }
        }
        if (!t){printf("No matching transition. Halting...\n"); break;}  // Halt if no matching transition

        // Execute transition: write symbol to tape
        machine->tape.head->symbol = t->write;

        // Move tape head according to direction
        if (t->direction == 'L') {  // Move left
            //printf("moveing left..\n");
            if (!machine->tape.head->left) {  // Expand tape if needed
                add_cell_to_tape(&machine->tape,t->direction);
            }
            machine->tape.head = machine->tape.head->left;
        } else if (t->direction == 'R') {  // Move right
            //printf("moveing right..\n");
            if (!machine->tape.head->right) {  // Expand tape if needed
                add_cell_to_tape(&machine->tape,t->direction);
            }
            machine->tape.head = machine->tape.head->right;
        }

        // transition_t to next state
        machine->current_state = t->next_state;
    }

    // Print final tape configuration from left to right
    print_tape(machine->tape);

    // Clean up allocated memory
    free_machine(machine);
    free(input);

    return 0;
}
