#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>

// Define maximum limits for our Turing machine components
#define MAX_STATES 255                 // Maximum number of states allowed
#define MAX_TRANSITIONS_PER_STATE 255  // Maximum transitions per state
#define STATE_NAME_LEN 50              // Maximum length for state names
#define MAX_INPUT MAX_STATES*MAX_TRANSITIONS_PER_STATE

// Structure to represent a transition rule
typedef struct {
    char read;                   // Symbol to read from tape
    char write;                  // Symbol to write to tape
    char direction;              // Movement direction (L/R/S for Left/Right/Stay)
    char next_state[STATE_NAME_LEN];  // State to transition to
} Transition;

// Structure to represent a state with its transitions
typedef struct {
    char name[STATE_NAME_LEN];         // Name of the state
    Transition transitions[MAX_TRANSITIONS_PER_STATE];  // List of transitions
    int num_transitions;               // Current number of transitions
} State;

// Structure for a cell in our tape (doubly linked list implementation)
typedef struct Cell {
    char symbol;            // Character stored in this cell
    struct Cell *left;      // Pointer to cell to the left
    struct Cell *right;     // Pointer to cell to the right
} Cell;

// Structure to manage the entire tape
typedef struct {
    Cell *head;             // Current position of the head
    Cell *leftmost;         // Leftmost cell in the tape
    Cell *rightmost;        // Rightmost cell in the tape
} Tape;

// Function to free all memory allocated for the tape
void free_tape(Tape *tape) {
    Cell *current = tape->leftmost;
    while (current) {
        Cell *next = current->right;  // Save next pointer before freeing
        free(current);
        current = next;
    }
}
char* read_stdin(void) {
    // Initial buffer size
    size_t buffer_size = 1024;
    size_t content_size = 0;
    char* buffer = malloc(buffer_size);
    
    if (buffer == NULL) {
        fprintf(stderr, "Failed to allocate initial buffer\n");
        return NULL;
    }

    // Read chunks until EOF
    char chunk[128];
    while (fgets(chunk, sizeof(chunk), stdin) != NULL) {
        size_t chunk_len = strlen(chunk);
        
        // Ensure buffer is large enough
        if (content_size + chunk_len + 1 > buffer_size) {
            buffer_size *= 2;
            char* new_buffer = realloc(buffer, buffer_size);
            
            if (new_buffer == NULL) {
                fprintf(stderr, "Failed to reallocate buffer\n");
                free(buffer);
                return NULL;
            }
            
            buffer = new_buffer;
        }
        
        // Copy chunk to buffer
        memcpy(buffer + content_size, chunk, chunk_len);
        content_size += chunk_len;
    }

    // Check for read errors
    if (ferror(stdin)) {
        fprintf(stderr, "Error reading from stdin\n");
        free(buffer);
        return NULL;
    }

    // Ensure null termination
    buffer[content_size] = '\0';
    
    // Shrink buffer to actual size
    char* final_buffer = realloc(buffer, content_size + 1);
    return final_buffer ? final_buffer : buffer;
}
int main() {
    char *input = read_stdin();
    //fgets(input, sizeof(input), stdin);   // Get user input
    //input[strcspn(input, "\n")] = 0;      // Remove newline character

    // Split input into three parts using | as delimiter
    char *initial_tape = strtok(input, "|");      // Get initial tape configuration
    char *start_state = strtok(NULL, "|");        // Get starting state
    char *transitions_part_ptr = strtok(NULL, "|");   // Get transitions section

    // Validate input format
    if (!initial_tape || !start_state || !transitions_part_ptr) {
        fprintf(stderr, "Invalid input format\n");
        return 1;
    }

    // Array to store all states
    State states[MAX_STATES];
    int num_states = 0;

    // Parse transitions section into individual transition rules
    int refcount = 0;
    char* transition[MAX_INPUT] = {NULL};
    transition[refcount] = strtok(transitions_part_ptr,";");
    while(transition[refcount]) {
    refcount++;
    transition[refcount] = strtok(NULL,";");
    }

    
    for (int z = 0; z < refcount; z++) {
        // Split transition into components using , as delimiter
        char *current_state = strtok(transition[z], ",");
        char *read_symbol = strtok(NULL, ",");
        char *write_symbol = strtok(NULL, ",");
        char *direction = strtok(NULL, ",");
        char *next_state = strtok(NULL, ",");

        // Validate transition format
        if (!current_state || !read_symbol || !write_symbol || !direction || !next_state) {
            fprintf(stderr, "Invalid transition: %s\n", transition[z]);
            return 1;
        }
        printf("Creating transition %s,%s,%s,%s,%s\n",current_state,read_symbol,write_symbol,direction,next_state);

        // Check if we already have this state in our list
        int found = 0;
        for (int i = 0; i <= num_states; i++) {
            printf("the number of states are: %d\n",num_states);
            printf("The following states are generated: %s\n",states[i].name);
            if (strcmp(states[i].name, current_state) == 0) {
                // Update existing state's transitions
                printf("transition found. Updating...\n");
                int j;
                for (j = 0; j < states[i].num_transitions; j++) {
                    // Check if transition for this symbol already exists
                    if (states[i].transitions[j].read == *read_symbol) {
                        // Update existing transition
                        states[i].transitions[j].write = *write_symbol;
                        states[i].transitions[j].direction = *direction;
                        strncpy(states[i].transitions[j].next_state, next_state, STATE_NAME_LEN);
                        break;
                    }
                }
                if (j == states[i].num_transitions) {
                    if (states[i].num_transitions >= MAX_TRANSITIONS_PER_STATE) {
                        fprintf(stderr, "Too many transitions for state %s\n", current_state);
                        return 1;
                    }
                    Transition t = {*read_symbol, *write_symbol, *direction};
                    strncpy(t.next_state, next_state, STATE_NAME_LEN);
                    states[i].transitions[states[i].num_transitions++] = t;
                }
                found = 1;
                break;
            }
        }

        // If state doesn't exist, create new state
        if (!found) {
            if (num_states >= MAX_STATES) {
                fprintf(stderr, "Too many states\n");
                return 1;
            }
            // Initialize new state
            printf("state doesn't exist. Creating...\n");
            State new_state;
            strncpy(new_state.name, current_state, STATE_NAME_LEN);
            new_state.num_transitions = 0;
            // Create first transition for this state
            Transition t = {*read_symbol, *write_symbol, *direction};
            strncpy(t.next_state, next_state, STATE_NAME_LEN);
            new_state.transitions[new_state.num_transitions++] = t;
            states[num_states++] = new_state;
            printf("There are %d many states created\n", num_states);
        }

        printf("transition var string: %s\n",transition[z]);
    }

    // Initialize tape using doubly linked list
    Tape tape = {0};
    Cell *prev = NULL;
    for (int i =0; initial_tape[i] != '\0'; i++){
        // Create new cell for each character in initial tape
        Cell *cell = malloc(sizeof(Cell));
        cell->symbol = initial_tape[i];
        cell->left = prev;
        cell->right = NULL;
        
        // Link previous cell to new cell
        if (prev) 
            prev->right = cell;
        else  // First cell becomes leftmost
            tape.leftmost = cell;
        
        prev = cell;
    }
    tape.rightmost = prev;   // Last cell becomes rightmost
    tape.head = tape.leftmost;  // Start at leftmost cell

    // Initialize current state with start state
    char current_state[STATE_NAME_LEN];
    strcpy(current_state, start_state);

    // Main Turing machine simulation loop
    while (1) {
        // Find current state in our states array
        State *cur_state = NULL;
        for (int i = 0; i < num_states; i++) {
            if (strcmp(states[i].name, current_state) == 0) {
                cur_state = &states[i];
                break;
            }
        }
        if (!cur_state){ printf("No state found. halting..\n"); break;}  // Halt if state not found

        // Get current symbol under head
        char read = tape.head->symbol;
        printf("reading symbol %c\n",read);
        Transition *t = NULL;
        
        // Find matching transition for current symbol
        printf("%d\n",cur_state->num_transitions);
        for (int i = 0; i < cur_state->num_transitions; i++) {
            if (cur_state->transitions[i].read == read) {
                t = &(cur_state->transitions[i]);
                printf("Found matching transition for symbol!\n");
                break;
            }
        }
        if (!t){printf("No matching transition. Halting...\n"); break;}  // Halt if no matching transition

        // Execute transition: write symbol to tape
       // printf("turning %s into %s",tape.head->symbol,t->write);
        tape.head->symbol = t->write;

        // Move tape head according to direction
        if (t->direction == 'L') {  // Move left
            if (!tape.head->left) {  // Expand tape if needed
                Cell *new_cell = malloc(sizeof(Cell));
                new_cell->symbol = 'B';  // Use 'B' as blank symbol
                new_cell->right = tape.head;
                new_cell->left = NULL;
                tape.head->left = new_cell;
                tape.leftmost = new_cell;  // Update leftmost pointer
            }
            tape.head = tape.head->left;
        } else if (t->direction == 'R') {  // Move right
            if (!tape.head->right) {  // Expand tape if needed
                Cell *new_cell = malloc(sizeof(Cell));
                new_cell->symbol = 'B';  // Use 'B' as blank symbol
                new_cell->left = tape.head;
                new_cell->right = NULL;
                tape.head->right = new_cell;
                tape.rightmost = new_cell;  // Update rightmost pointer
            }
            tape.head = tape.head->right;
        }

        // Transition to next state
        strcpy(current_state, t->next_state);
    }

    // Print final tape configuration from left to right
    Cell *current = tape.leftmost;
    while (current) {
        putchar(current->symbol);
        current = current->right;
    }
    putchar('\n');

    // Clean up allocated memory
    free_tape(&tape);
    free(input);

    return 0;
}
