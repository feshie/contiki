#ifndef FILENAMES_OLD_H
#define FILENAMES_OLD_H

	#include <stdint.h>

	uint8_t get_next_write_filename(char *filename);
	uint8_t get_next_read_filename(char *filename);
	#define FILENAME_LENGTH 8

#endif
