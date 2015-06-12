/*
 * list the number of files in th ecoffee file system
 */

/**
 * \file
 *         Basic test for CFS/Coffee.
 * \author
 *         Kirk Martinez, University of Southampton
 */

#include "contiki.h"
#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"
#include "lib/crc16.h"
#include "lib/random.h"

#include <stdio.h>
#include <string.h>

PROCESS(listcoffee_process, "CFS/Coffee dir list process");
AUTOSTART_PROCESSES(&listcoffee_process);

/* returns the number of files in the directory on Flash
*/
int coffee_du(int *filec, int *bytes)
{
struct cfs_dir dir;
struct cfs_dirent dirent;
int dirp;
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

// no idea how to close dir
//cfs_closedir(&dirp);
*bytes = used;
*filec = count;
return(0);
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(listcoffee_process, ev, data)
{
int c;
int used;
int count;
  PROCESS_BEGIN();

  c = coffee_du(&count, &used );
  printf("%d files %d bytes used\n",count, used);
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
