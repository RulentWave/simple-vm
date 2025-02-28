#include <pthread.h>
#include <stdlib.h>

typedef struct Par_Map_Args_t {
	void* array;
	size_t elem_size;
	int start_idx;
	unsigned num_elements;
	void (*func)(void*);
	void* func_arg;
} par_map_args_t;

typedef struct par_map_elem_arg_t {
	void* element;
	void* func_arg;
} par_map_elem_arg_t;

static void* thread_function(void* arg) {
	par_map_args_t* args = (par_map_args_t*)arg;
	char* array_base = (char*)args->array;
	for (int i = 0; i < args->num_elements; ++i) {
		void* element = array_base + (args->start_idx + i) * args->elem_size;
		args->func(element);
	}
	return NULL;
}
static void* thread_function_arg(void* arg) {
	par_map_args_t* args = (par_map_args_t*)arg;
	char* array_base = (char*)args->array;
	for (int i = 0; i < args->num_elements; ++i) {
		void* element = array_base + (args->start_idx + i) * args->elem_size;
		par_map_elem_arg_t send;
		send.element = element;
		send.func_arg = args->func_arg;
		args->func(&send);
	}
	return NULL;
}

void par_map(par_map_args_t arg, unsigned num_threads) {
	if (num_threads <= 0 || arg.num_elements <= 0 || !arg.array || !arg.func || arg.elem_size == 0) {
		return;
	}

	if (num_threads > arg.num_elements) {
		num_threads = arg.num_elements;
	}

	pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
	par_map_args_t* args = malloc(num_threads * sizeof(par_map_args_t));
	if (!threads || !args) {
		free(threads);
		free(args);
		return;
	}

	int base = arg.num_elements / num_threads;
	int remainder = arg.num_elements % num_threads;
	int current_start = 0;

	if (!arg.func_arg) {
		for (int i = 0; i < num_threads; ++i) {
			int num_elements = base + (i < remainder ? 1 : 0);
			args[i].array = arg.array;
			args[i].elem_size = arg.elem_size;
			args[i].start_idx = current_start;
			args[i].num_elements = arg.num_elements;
			args[i].func = arg.func;
			args[i].func_arg = arg.func_arg;

			pthread_create(&threads[i], NULL, thread_function, &args[i]);
			current_start += num_elements;
		}
	} else {
		for (int i = 0; i < num_threads; ++i) {
			int num_elements = base + (i < remainder ? 1 : 0);
			args[i].array = arg.array;
			args[i].elem_size = arg.elem_size;
			args[i].start_idx = current_start;
			args[i].num_elements = arg.num_elements;
			args[i].func = arg.func;
			args[i].func_arg = arg.func_arg;

			pthread_create(&threads[i], NULL, thread_function_arg, &args[i]);
			current_start += num_elements;
		}
	}

	for (int i = 0; i < num_threads; ++i) {
		pthread_join(threads[i], NULL);
	}

	free(threads);
	free(args);
}
