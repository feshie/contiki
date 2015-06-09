
// General
#include "contiki.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cfs/cfs.h"

#include "filenames-old.h"
#ifdef SPI_LOCKING
#include "dev/cc1120.h"
#include "dev/cc1120-arch.h"
#endif
#include "platform-conf.h"

//#define FILEDEBUG
#ifdef FILEDEBUG
    #define FILEDEBUG(...) printf(__VA_ARGS__)
#else
    #define FILEDEBUG(...)
#endif


/*
 * Returns the filename that is to be read for POSTing
 * Returns NULL if no files can be POSTed
 */
uint8_t
get_next_read_filename(char *filename)
{
  uint8_t ret_code;
  struct cfs_dirent dirent;
  struct cfs_dir dir;
  ret_code = 1;
#ifdef SPI_LOCKING
  LPRINT("LOCK:read filename\n");
  NETSTACK_MAC.off(0);
  cc1120_arch_interrupt_disable();
  CC1120_LOCK_SPI();
#endif
  if(cfs_opendir(&dir, "/") == 0) {
    while(cfs_readdir(&dir, &dirent) != -1) {
      if(strncmp(dirent.name, "r_", 2) == 0) {
        strcpy(filename, dirent.name);
        ret_code = 1;
      }
    }
  }else{
    FILEDEBUG("[NEXT] No file found. NULL\n");
    ret_code=  0;
  }
#ifdef SPI_LOCKING
  LPRINT("UNLOCK: read filename\n");
  CC1120_RELEASE_SPI();
  cc1120_arch_interrupt_enable();
  NETSTACK_MAC.on();
#endif
  return ret_code;
}

/*
 * Returns the filename of the next file where it is safe to store `length` bytes of data
 */
uint8_t
get_next_write_filename(char *filename)
{
  FILEDEBUG("get_next_write_filename\n");
  struct cfs_dirent dirent;
  struct cfs_dir dir;
  uint16_t file_num;
  uint16_t max_num;
  uint8_t ret_code;
  ret_code = 1;
  file_num = 0;
  max_num = 0;

  filename[0] = 'r';
  filename[1] = '_';
#ifdef SPI_LOCKING
  LPRINT("LOCK:write filename\n");
  NETSTACK_MAC.off(0);
  cc1120_arch_interrupt_disable();
  CC1120_LOCK_SPI();
#endif

  if(cfs_opendir(&dir, "/") == 0) {
    FILEDEBUG("\tOpened folder\n");
    while(cfs_readdir(&dir, &dirent) != -1) {
      if(strncmp(dirent.name, "r_", 2) == 0) {
        file_num = atoi(dirent.name + 2);
        if(file_num > max_num) {
          max_num = file_num;
          //file_size = (uint16_t)dirent.size;
          FILEDEBUG("Filename %d found\n", file_num);
        }
        FILEDEBUG("\tMax: %d Filenum: %d\n", max_num, file_num);
      }
    }
    if(max_num == 0) {
      FILEDEBUG("\tNo previous files found\n");
      filename[2] = '1';
      filename[3] = 0;
    }else{
      FILEDEBUG("\t Previous file %d\n", max_num);
      itoa(max_num + 1, filename + 2, 10);
    }

    FILEDEBUG("Returning %s\n", filename);
    ret_code =  1;
  }else{
    FILEDEBUG("[ERROR] UNABLE TO OPEN ROOT DIRECTORY!!!\n");
    ret_code = 0;
  }
#ifdef SPI_LOCKING
  LPRINT("UNLOCK: write filename\n");
  CC1120_RELEASE_SPI();
  cc1120_arch_interrupt_enable();
  NETSTACK_MAC.on();
#endif
  return ret_code;
}
