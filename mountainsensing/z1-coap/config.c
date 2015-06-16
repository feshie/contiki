
#include "config.h"


#if SensorConfig_size > PostConfig_size
    #define CONFIG_BUF_SIZE (SensorConfig_size + 4)
#else
    #define CONFIG_BUF_SIZE (POSTConfig_size + 4)
#endif

//#define CONFIG_DEBUG
#ifdef CONFIG_DEBUG
    #define CPRINT(...) printf(__VA_ARGS__)
#else
    #define CPRINT(...)
#endif


void 
print_sensor_config(SensorConfig *conf)
{
  uint8_t i;
  CPRINT("\tInterval = %d\n", (unsigned int)conf->interval);
  CPRINT("\tADC1: ");
  if (conf->hasADC1 == 1){
    CPRINT("yes\n");
  }else{
    CPRINT("no\n");
  }
  CPRINT("\tADC2: ");
  if (conf-> hasADC2){
    CPRINT("yes\n");
  }else{
    CPRINT("no\n");
  }
  CPRINT("\tRain: ");
  if (conf-> hasRain){
    CPRINT("yes\n");
  }else{
    CPRINT("no\n");
  }
  CPRINT("\tAVRs: ");
  if(conf->avrIDs_count ==0 ){
    CPRINT("NONE\n");
  }else{
    for(i=0; i < conf->avrIDs_count; i++){
      CPRINT("%02x", (int)conf->avrIDs[i] & 0xFF);
      if(i < conf->avrIDs_count -1){
        CPRINT(", ");
      }else{
        CPRINT("\n");
      }
    }
  }
}

uint8_t 
set_config(void* pb, uint8_t config)
{
    uint8_t cfg_buf[CONFIG_BUF_SIZE];
    pb_ostream_t ostream;
    int write;
    uint8_t ret_code;
    bool encode_status;

    memset(cfg_buf, 0, CONFIG_BUF_SIZE);
    ostream = pb_ostream_from_buffer(cfg_buf, CONFIG_BUF_SIZE);
#ifdef SPI_LOCKING
      LPRINT("LOCK: set conf\n");
      NETSTACK_MAC.off(0);
      cc1120_arch_interrupt_disable();
      CC1120_LOCK_SPI();
#endif
    if(config == SAMPLE_CONFIG) {
        encode_status = pb_encode_delimited(&ostream, SensorConfig_fields, (SensorConfig *)pb);
        cfs_remove("sampleconfig");
        write = cfs_open("sampleconfig", CFS_WRITE);
        CPRINT("Saving the following details to config file\n");
        print_sensor_config((SensorConfig *)pb);
    }else{
        CPRINT("UNKNOWN CONFIG TYPE. UNABLE TO WRITE FILE!\n");
        encode_status = 0;
        ret_code = 2;
    }
    if(encode_status){
        if(write != -1) {
            cfs_write(write, cfg_buf, ostream.bytes_written);
            cfs_close(write);
            CPRINT("[WCFG] Writing %d bytes to config file\n", ostream.bytes_written);
           ret_code = 0;
        } else {
           CPRINT("[WCFG] ERROR: could not write to disk\n");
           ret_code = 1;
        }
    }else{
      CPRINT("Failed to encode file. Leaving config as is\n");
      ret_code = 2;
    }
#ifdef SPI_LOCKING
    LPRINT("UNLOCK: set config\n");
    CC1120_RELEASE_SPI();
    cc1120_arch_interrupt_enable();
    NETSTACK_MAC.on();
#endif
    return ret_code;
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
    uint8_t ret_code;

    memset(cfg_buf, 0, CONFIG_BUF_SIZE);
#ifdef SPI_LOCKING
      LPRINT("LOCK: get conf\n");
      NETSTACK_MAC.off(0);
      cc1120_arch_interrupt_disable();
      CC1120_LOCK_SPI();
#endif
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

      pb_decode_delimited(&istream, SensorConfig_fields, (SensorConfig *)pb);
      ret_code =  0;
    } else {
        CPRINT("[RCFG] ERROR: could not read from disk\n");
        ret_code =  1;
    }
#ifdef SPI_LOCKING
    LPRINT("UNLOCK: get conf\n");
    CC1120_RELEASE_SPI();
    cc1120_arch_interrupt_enable();
    NETSTACK_MAC.on();
#endif
    return ret_code;
}
