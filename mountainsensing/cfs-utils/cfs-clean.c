/**
 * \file
 * Clean the Coffee file system up.
 * Removes every file except for:
 *  sampleconfig
 *  rNUMBER
 * \author
 *         Arthur Fabre
 */

#include "contiki.h"
#include "cfs/cfs.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define CONFIG "sampleconfig"

#define FILENAME_PREFIX "r"

PROCESS(listcoffee_process, "CFS/Coffee process");
AUTOSTART_PROCESSES(&listcoffee_process);

bool is_num(char c) {
    return c > 47 && c < 58;
}

void coffee_clean() {
    struct cfs_dirent dirent;
    struct cfs_dir dir;

    printf("Cleaning filesystem\n");

    if (cfs_opendir(&dir, "/") == 0) {
        printf("\tOpened folder\n");

        while (cfs_readdir(&dir, &dirent) != -1) {
            printf("Found file %s\n", dirent.name);

            // If it's a config file, ignore it
            if (strncmp(dirent.name, CONFIG, strlen(CONFIG)) == 0) {
                printf("It's a config file, skipping\n");
                continue;
            }

            // If it's a reading ignore it
            // Check the name is at least two long, starts with r, and that the next char is a digit
            if (strlen(dirent.name) > 1 && strncmp(dirent.name, FILENAME_PREFIX, strlen(FILENAME_PREFIX)) == 0 && is_num(dirent.name[1])) {
                printf("It's a sample file, skipping\n");
                continue;
            }

            // Delete everything else
            if (cfs_remove(dirent.name) == -1) {
                printf("Failed to delete file %s\n", dirent.name);
            } else {
                printf("Removed file %s\n", dirent.name);
            }
        }
    }

    printf("Done cleaning file system\n");
}

PROCESS_THREAD(listcoffee_process, ev, data) {
    PROCESS_BEGIN();

    coffee_clean();

    PROCESS_END();
}
