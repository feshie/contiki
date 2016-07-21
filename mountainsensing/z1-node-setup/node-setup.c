#include <stdio.h>

#include "contiki.h"
#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"
#include "ms-io.h"

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
  printf("Available files:\n");
  while(cfs_readdir(&dir, &dirent) == 0) {
    printf("  %s (%lu bytes)\n", dirent.name, (unsigned long)dirent.size);
  }

  cfs_closedir(&dir);

  return 1;
}

PROCESS_THREAD(example_coffee_process, ev, data)
{
  PROCESS_BEGIN();

  printf("\n\nNode Setup\n");

#if NEED_FORMATTING
  printf("Formatting flash... ");
  cfs_coffee_format();
  printf("Done\n");
#endif
  
  PROCESS_PAUSE();

  if(dir_test() == 0) {
    printf("dir test failed\n");
  }

  // Reset the reboot counter
  printf("Reseting reboot counter... ");
  ms_reset_reboot();
  printf("Done\n");

  printf("\nNode setup complete\n");

  PROCESS_END();
}
