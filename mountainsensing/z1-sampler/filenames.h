#ifndef FILENAMES_H
#define FILENAMES_H

#define FILENAME_PREFIX 'r'
#define FILENAME_LENGTH 8
void filenames_init(void);
char* filenames_next_read(char*);
char* filenames_next_write(char*);
void filenames_delete(char*);
void filenames_refresh(void);

#endif
