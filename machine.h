#define STATE_NAME_LEN 63
#define MAX_TRANSITIONS_PER_STATE 255
#define MAX_STATES 255

typedef struct Transition_t {
    char read;                   // Symbol to read from tape
    char write;                  // Symbol to write to tape
    char direction;              // Movement direction (L/R/S for Left/Right/Stay)
    char next_state_name[STATE_NAME_LEN];  // State to transition to
    struct State_t* next_state;
} transition_t;

// Structure to represent a state with its transitions
typedef struct State_t {
    char name[STATE_NAME_LEN];         // Name of the state
    transition_t transitions[MAX_TRANSITIONS_PER_STATE];  // List of transitions
    int num_transitions;               // Current number of transitions
} state_t;

// Structure for a cell in our tape (doubly linked list implementation)
typedef struct Cell_t {
    char symbol;            // Character stored in this cell
   struct Cell_t *left;      // Pointer to cell to the left
   struct Cell_t *right;     // Pointer to cell to the right
} cell_t; 

// Structure to manage the entire tape
typedef struct Tape_t {
    cell_t *head;             // Current position of the head
    cell_t *leftmost;         // Leftmost cell in the tape
    cell_t *rightmost;        // Rightmost cell in the tape
} tape_t;
typedef struct Machine_t {
    tape_t tape;
    state_t states[MAX_STATES];
    state_t* current_state;
    char initial_state_name[STATE_NAME_LEN];
    int num_states;
} machine_t;


