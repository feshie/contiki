#include "filenames.h"

#include "contiki.h"
#include "cfs/cfs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform-conf.h"

#ifdef SPI_LOCKING
    #include "dev/cc1120.h"
    #include "dev/cc1120-arch.h"
#endif

//#define FILENAME_DEBUG
#ifdef FILENAME_DEBUG
    #define FPRINT(...) printf(__VA_ARGS__)
#else
    #define FPRINT(...) 
#endif

static uint16_t number;

void 
filenames_refresh(void){
    struct cfs_dirent dirent;
    struct cfs_dir dir;
    uint16_t file_num;
    uint16_t max_num;
    max_num = 0;
    FPRINT("Refreshing filename cache\n");
#ifdef SPI_LOCKING
    LPRINT("LOCK:refresh filename\n");
    NETSTACK_MAC.off(0);
    cc1120_arch_interrupt_disable();
    CC1120_LOCK_SPI();
#endif
    if(cfs_opendir(&dir, "/") == 0) {
        FPRINT("\tOpened folder\n");
        while(cfs_readdir(&dir, &dirent) != -1) {
            if(strncmp(dirent.name, FILENAME_PREFIX, 1) == 0) {
                file_num = atoi(dirent.name + 1);
                FPRINT("Filename %d found\n", file_num);
                FPRINT("\tMax: %d Filenum: %d\n", max_num, file_num);
                if(file_num > max_num) {
                    max_num = file_num;
                }
            }
        }
        if(max_num == 0) {
            printf("\tNo previous files found\n");
            number = 0;
        }else{
            printf("\tPrevious files found.  Highest number = %d\n", max_num);
            number = max_num;
        }
    }
#ifdef SPI_LOCKING
    LPRINT("UNLOCK: refresh filename\n");
    CC1120_RELEASE_SPI();
    cc1120_arch_interrupt_enable();
    NETSTACK_MAC.on();
#endif
}

void 
filenames_init(void){
    FPRINT("Initing filename system\n");
    filenames_refresh();
}

char*
filenames_next_read(char* filename){
    filename[0] = FILENAME_PREFIX;
    itoa(number, filename + 1, 10);
    FPRINT("Next read filename = %s\n", filename);
    return filename;
}

char*
filenames_next_write(char * filename){
    uint16_t next;
    next = ++number;
    filename[0] = FILENAME_PREFIX;
    itoa(next, filename +1, 10);
    FPRINT("Next write filename = %s\n", filename);
    return filename;
}

void
filenames_delete(char* filename){
    char fname[FILENAME_LENGTH];
    if (strcmp(filename,filenames_next_read(fname)) == 0){
        cfs_remove(filename);
        FPRINT("File deleted\n");
        number--;
        FPRINT("Number decremented\n");
    }else{
        printf("Number mismatch not deleting\n");
        filenames_refresh();
    }
}
