#include <stdio.h>
#include "contiki.h"
#include "filenames.h"
#include "cfs/cfs.h"

#define FILENAME_DEBUG
#define DELAY_TIME CLOCK_SECOND

PROCESS (filename_test_process, "Filename test  process");
AUTOSTART_PROCESSES (&filename_test_process);

/*---------------------------------------------------------------------------*/
static struct etimer et;

PROCESS_THREAD (filename_test_process, ev, data)
{
    PROCESS_BEGIN ();
    static uint16_t i = 0;
    static char filename[FILENAME_LENGTH];
    static int16_t write;
    filenames_init();

    filenames_next_read(filename);
    while (1){
 
        filenames_next_write(filename);
        write = cfs_open(filename, CFS_WRITE);
        cfs_write(write, "123", 3);
        cfs_close(write);
        filenames_next_read(filename);
        filenames_next_write(filename);
        printf("i=%d\n", i);
        if((i % 10) == 0 && (i > 9)){
            filenames_delete(filename);
        }
        if ((i % 20) == 0 && (i > 9)){
            printf("D");
            filenames_delete("r40");
        }


        etimer_set(&et, DELAY_TIME);          // Set the timer
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));  // wait for its expiration
        i=i+1;
        printf("********\n");
    }
    PROCESS_END ();
}
