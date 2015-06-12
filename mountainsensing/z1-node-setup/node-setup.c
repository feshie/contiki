#include <stdio.h>

#include "contiki.h"
#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"
#include "dev/reset-sensor.h"     // Include sensor driver

PROCESS(example_coffee_process, "Node Reset");
AUTOSTART_PROCESSES(&example_coffee_process);

/* Formatting is needed if the storage device is in an unknown state; 
   e.g., when using Coffee on the storage device for the first time. */
#ifndef NEED_FORMATTING
#define NEED_FORMATTING 1
#endif

static int
dir_test(void)
{
  struct cfs_dir dir;
  struct cfs_dirent dirent;

  /* Coffee provides a root directory only. */
  if(cfs_opendir(&dir, "/") != 0) {
    printf("failed to open the root directory\n");
    return 0;
  }

  /* List all files and their file sizes. */
  printf("Available files\n");
  while(cfs_readdir(&dir, &dirent) == 0) {
    printf("%s (%lu bytes)\n", dirent.name, (unsigned long)dirent.size);
  }

  cfs_closedir(&dir);

  return 1;
}

PROCESS_THREAD(example_coffee_process, ev, data)
{
  PROCESS_BEGIN();

#if NEED_FORMATTING
  cfs_coffee_format();
#endif
  
  PROCESS_PAUSE();

  if(dir_test() == 0) {
    printf("dir test failed\n");
  }

  reset_counter_reset();
  printf ("COUNT = %d\n", reset_sensor.value(0));

  printf("Node reset complete");

  PROCESS_END();
}
