/*
 * list the number of files in th ecoffee file system
 */

/**
 * \file
 *         Basic test for CFS/Coffee.
 * \author
 *         Kirk Martinez, University of Southampton
 * 	   Philip Basford, pjb@ecs.soton.ac.uk
 */

#include "contiki.h"
#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"
#include "lib/crc16.h"
#include "lib/random.h"

#include <stdio.h>
#include <string.h>

#define SPI_LOCKING

#ifdef SPI_LOCKING
        #include "dev/cc1120.h"
        #include "dev/cc1120-arch.h"
#endif
#define DATA_BUFFER_LENGTH 200

PROCESS(listcoffee_process, "CFS/Coffee dir list process");
AUTOSTART_PROCESSES(&listcoffee_process);


int8_t
load_file(char *data_buffer, char *filename)
{
    int fd;
    uint8_t data_length;
#ifdef SPI_LOCKING
    NETSTACK_MAC.off(0);
    cc1120_arch_interrupt_disable();
    CC1120_LOCK_SPI();
#endif
    fd = cfs_open(filename, CFS_READ);
    if(fd >= 0){
        data_length = cfs_read(fd, data_buffer, DATA_BUFFER_LENGTH);
        cfs_close(fd);
    }else{
        data_length = 0;
    }
#ifdef SPI_LOCKING
    CC1120_RELEASE_SPI();
    cc1120_arch_interrupt_enable();
    NETSTACK_MAC.on();
#endif
    return data_length;
}



/* returns the number of files in the directory on Flash
*/
int coffee_cat()
{
struct cfs_dir dir;
struct cfs_dirent dirent;
int count = 0;
int used = 0;

if(cfs_opendir(&dir, "/") == 0) {
   while(cfs_readdir(&dir, &dirent) != -1) {
     printf("%s %ld\n",
            dirent.name, (long)dirent.size);
	count++;
	used += (long)dirent.size;
   }
   //cfs_closedir(&dir);
 }
else {
    printf("opendir failed\n");
 }

return(0);
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(listcoffee_process, ev, data)
{
  PROCESS_BEGIN();

  coffee_cat();
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
