/*
 * FILL THE 2MB flash just to see
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

PROCESS(listcoffee_process, "CFS/Coffee process");
AUTOSTART_PROCESSES(&listcoffee_process);

int coffee_fill()
{
struct cfs_dir dir;
struct cfs_dirent dirent;
static int bufsize;
char buf[500]; // data to write
char filename[80];
bufsize=500;
int dirp;
int count = 0;
int used = 0;
int fd;
int bytes;
int full;
full =0;

cfs_coffee_format();
printf("formatted flash\n");
while( !full){
	sprintf(filename,"f%d",count);
    fd = cfs_open(filename, CFS_WRITE);

    if (fd < 0) {
        printf("Failed to open file %s for writing\n", filename);
        full = 1;
    }

    bytes = cfs_write(fd, buf, bufsize);

    if (bytes != bufsize) {
        printf("Failed to write file to %s, wrote %d bytes\n", filename, bytes);
        full = 1;
    }

    cfs_close(fd);
	printf("%d\n",count++);
	}
return(0);
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(listcoffee_process, ev, data)
{
int c;
int used;
int count;
  PROCESS_BEGIN();

  c = coffee_fill();
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
