#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
//Machine state definitions:
#define NONE 'n'
#define LEFT 'l'
#define RIGHT 'r'
#define MAXSTATES 200

//Symbol definitions
#define BLANK ' '

//Table instructions
#define WRITE 0
#define MOVE 1
#define SET 2
struct Cell {
	struct Cell* prev;
	char this;
	struct Cell* next;
};
struct Machine{
	int numstates;
	char table[MAXSTATES][2];
	int currentstate;
	struct Cell* currentcellptr;
};
struct Cell* cell_append(struct Cell* currentptr){
	while (currentptr != NULL){
		currentptr = currentptr->next;
	}
	currentptr->next = malloc(sizeof(struct Cell));
	currentptr->next->next = NULL;
	currentptr->next->prev = currentptr;
	currentptr->next->this = BLANK;
	return currentptr->next;
}
void tape_free(struct Cell* currentptr){
	while (currentptr->prev != NULL){
		currentptr = currentptr->prev;
	}
	while (currentptr->next != NULL){
		currentptr = currentptr->next;
		free(currentptr->prev);
	}
	free(currentptr);
}
struct Cell* tape_constructor (){
	struct Cell* firstcell = malloc(sizeof(struct Cell));
	firstcell->prev = NULL;
	firstcell->next = NULL;
	firstcell->this = BLANK;
	return firstcell;
}

struct Cell* tape_intake(struct Cell* currentptr){
	char input;
	while (read(STDIN_FILENO, &input, 1) > 0){
		currentptr->this = input;
		currentptr = cell_append(currentptr);
		}
	while (currentptr->prev != NULL){
		currentptr = currentptr->prev;
	}
	return currentptr;
	
}
struct Machine getstates(struct Cell* currentptr){
	struct Machine machine;
	machine.numstates = 0;
	machine.currentstate = 0;
	machine.currentcellptr = currentptr;

	
	while (currentptr->next != NULL){
		if (currentptr->this == BLANK){ //count the number of states and assign the WRITE MOVE SET table for that state
		machine.table [machine.numstates] [WRITE] = currentptr->next->this;	
		machine.table [machine.numstates] [MOVE] = currentptr->next->next->this;
		machine.table [machine.numstates] [SET] = currentptr->next->next->next->this;
		machine.numstates++;
		}
		if (machine.numstates == MAXSTATES)
			break;
		currentptr = currentptr->next;
	}

	return machine;


}

struct Machine transition(struct Machine machine){
	machine.currentcellptr->this = machine.table [machine.currentstate] [WRITE];
	machine.currentstate = machine.table [machine.currentstate] [SET];
	switch (machine.table [machine.currentstate] [MOVE]){
		case LEFT:
			machine.currentcellptr = machine.currentcellptr->prev;
			break;
		case RIGHT:
			machine.currentcellptr = machine.currentcellptr->next;
			break;
		default:
			break;
	}


}




int main () {
	struct Cell* firstcellptr = tape_intake(tape_constructor());
	struct Machine machine = getstates(firstcellptr);
	while (machine.currentstate != machine.numstates){
		machine = transition(machine);
	}

	tape_free(firstcellptr);
}
