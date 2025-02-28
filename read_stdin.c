char *read_stdin(void) {
	// Initial buffer size
	size_t buffer_size = 1024;
	size_t content_size = 0;
	char *buffer = malloc(buffer_size);

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
			char *new_buffer = realloc(buffer, buffer_size);

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
	char *final_buffer = realloc(buffer, content_size + 1);
	return final_buffer ? final_buffer : buffer;
}
