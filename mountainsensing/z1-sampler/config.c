
#include "config.h"

// Config
#include "settings.pb.h"
#include "readings.pb.h"

// Protobuf
#include "dev/pb_decode.h"
#include "dev/pb_encode.h"

#include "cfs/cfs.h"
#include "contiki.h"

#include <stdlib.h>
#include <stdio.h>



#if SensorConfig_size > PostConfig_size
    #define CONFIG_BUF_SIZE (SensorConfig_size + 4)
#else
    #define CONFIG_BUF_SIZE ([POSTConfig_size + 4)
#endif

#define CONFIG_DEBUG

#ifdef CONFIG_DEBUG 
    #define CPRINT(...) printf(__VA_ARGS__)
#else
    #define DPRINT(...)
#endif

/*
 * Sets the Sampler configuration (writes it to flash).
 * Returns 0 upon success, 1 on failure
 */
uint8_t 
set_config(void* pb, uint8_t config)
{
    uint8_t cfg_buf[CONFIG_BUF_SIZE];
    pb_ostream_t ostream;
    int write;

    memset(cfg_buf, 0, CONFIG_BUF_SIZE);
    ostream = pb_ostream_from_buffer(cfg_buf, CONFIG_BUF_SIZE);
    if(config == SAMPLE_CONFIG) {
        pb_encode_delimited(&ostream, SensorConfig_fields, (SensorConfig *)pb);
        cfs_remove("sampleconfig");
        write = cfs_open("sampleconfig", CFS_WRITE);
    } else {
       pb_encode_delimited(&ostream, POSTConfig_fields, (POSTConfig *)pb);
       cfs_remove("commsconfig");
       write = cfs_open("commsconfig", CFS_WRITE);
   }
    if(write != -1) {
        cfs_write(write, cfg_buf, ostream.bytes_written);
        cfs_close(write);
        CPRINT("[WCFG] Writing %d bytes to config file\n", ostream.bytes_written);
       return 0;
    } else {
       CPRINT("[WCFG] ERROR: could not write to disk\n");
       return 1;
    }
}


/*
 * Get the Sampler config (reads it from flash).
 * Returns 0 upon success, 1 upon failure
 */
uint8_t 
get_config(void* pb, uint8_t config)
{
    int read;
    uint8_t cfg_buf[CONFIG_BUF_SIZE];
    pb_istream_t istream;

    memset(cfg_buf, 0, CONFIG_BUF_SIZE);
  
   if(config == SAMPLE_CONFIG) {
        CPRINT("[RCFG] Opening `sampleconfig`\n");
        read = cfs_open("sampleconfig", CFS_READ);
        CPRINT("[RCFG] Opened\n");
    } else {
        CPRINT("[RCFG] Opening `commsconfig`\n");
        read = cfs_open("commsconfig", CFS_READ);
    }
    CPRINT("[RCFG] Attmepting to read\n");
    if(read != -1) {
      CPRINT("[RCFG] Reading...\n");
      cfs_read(read, cfg_buf, CONFIG_BUF_SIZE);
      cfs_close(read);

      istream = pb_istream_from_buffer(cfg_buf, CONFIG_BUF_SIZE);

      CPRINT("[RCFG] Bytes left = %d\n", istream.bytes_left);

      if(config == SAMPLE_CONFIG) {
          pb_decode_delimited(&istream, SensorConfig_fields, (SensorConfig *)pb);
      } else {
          pb_decode_delimited(&istream, POSTConfig_fields, (POSTConfig *)pb);
      }
      return 0;
    } else {
        CPRINT("[RCFG] ERROR: could not read from disk\n");
        return 1;
    }
}