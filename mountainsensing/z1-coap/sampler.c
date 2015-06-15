
#include "sampler.h"

PROCESS(sample_process, "Sample Process");

//#define SENSEDEFBUG
#ifdef SENSEDEFBUG
    #define SPRINT(...) printf(__VA_ARGS__)
#else
    #define SPRINT(...)
#endif

//#define AVRDEFBUG
#ifdef AVRDEFBUG
    #define AVRDPRINT(...) SPRINT(__VA_ARGS__)
#else
    #define AVRDPRINT(...)
#endif

//#define SENSE_ON /*Do not turn sensor power off */

static SensorConfig sensor_config;
static POSTConfig POST_config;
static process_event_t protobuf_event;
//static struct psock ps;
//static uint8_t psock_buffer[PSOCK_BUFFER_LENGTH];


void
refreshSensorConfig(void){
    if(get_config(&sensor_config, SAMPLE_CONFIG) == 1){ 
        // Config file does not exist! Use default and set file
      	SPRINT("No Sensor config found\n");
        sensor_config.interval = SENSOR_INTERVAL;
        sensor_config.avrIDs_count = SENSOR_AVRIDS_COUNT;
        sensor_config.hasADC1 = SENSOR_HASADC1;
        sensor_config.hasADC2 = SENSOR_HASADC2;
        sensor_config.hasRain = SENSOR_HASRAIN;
        set_config(&sensor_config, SAMPLE_CONFIG);
    }else{
      	SPRINT("Sensor config loaded\n");
    }
}






PROCESS_THREAD(sample_process, ev, data)
{
    //sample variables
    static struct etimer sample_timer;
    static uint8_t pb_buf[Sample_size];
    static Sample sample;
    static int i;
#ifndef SAMPLE_SEND
    static int fd;
    static char filename[FILENAME_LENGTH];
#endif
    static uint8_t avr_id;
    static struct ctimer avr_timeout_timer;
    static uint8_t avr_recieved;
    static uint8_t avr_retry_count;
    static pb_ostream_t ostream;

    //poster variables

    PROCESS_BEGIN();
    refreshSensorConfig();
    refreshPosterConfig();
    filenames_init();
    avr_recieved = 0;
    avr_retry_count = 0;

    protobuf_event = process_alloc_event();
    protobuf_register_process_callback(&sample_process, protobuf_event) ;
    SPRINT("Refreshed Sensor config to:\n"); 
    print_sensor_config(&sensor_config);

#ifdef SENSE_ON
    ms1_sense_on();
    SPRINT("Sensor power permanently on\n");
#endif

#ifdef SAMPLE_SEND
    SPRINT("<<<<< SAMPLE AND SEND ENABLED >>>>>\n");
#endif

    SENSORS_ACTIVATE(event_sensor);
    SPRINT("[SAMP] Sampling sensors activated\n");
    while(1){
        etimer_set(&sample_timer, CLOCK_SECOND * (sensor_config.interval - (get_time() % sensor_config.interval)));
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sample_timer));
        ms1_sense_on();
        sample.time = get_time();

        sample.batt = get_sensor_batt();
        sample.has_batt = 1;

        SENSORS_ACTIVATE(temperature_sensor);
        sample.temp = get_sensor_temp();
        sample.has_temp = 1;
        SENSORS_DEACTIVATE(temperature_sensor);

        sample.accX = get_sensor_acc_x();
        sample.accY = get_sensor_acc_y();
        sample.accZ = get_sensor_acc_z();

        sample.has_accX = 1;
        sample.has_accY = 1;
        sample.has_accZ = 1;

        if(sensor_config.hasADC1) {
            sample.has_ADC1 = 1;
            sample.ADC1 = get_sensor_ADC1();
        }
        if(sensor_config.hasADC2) {

            sample.has_ADC2 = 1;
            sample.ADC2 = get_sensor_ADC2();
        }
        if(sensor_config.hasRain) {
            sample.has_rain = 1;
            sample.rain = get_sensor_rain();
        }
      
        AVRDPRINT("[SAMP][AVR] number: %d\n", sensor_config.avrIDs_count);
        for(i=0; i < sensor_config.avrIDs_count; i++){
            avr_id = sensor_config.avrIDs[i] & 0xFF; //only use 8bits
            AVRDPRINT("[SAMP][AVR] using ID: %d\n", avr_id);
            avr_recieved = 0;
            avr_retry_count = 0;
            data = NULL;
            do{
                protobuf_send_message(avr_id, PROTBUF_OPCODE_GET_DATA, NULL, (int)NULL);
                AVRDPRINT("Sent message %d\n", i);
                i = i+1;
                ctimer_set(&avr_timeout_timer, CLOCK_SECOND * AVR_TIMEOUT_SECONDS, avr_timer_handler, NULL);
                PROCESS_YIELD_UNTIL(ev == protobuf_event);
                if(data != NULL){
                    AVRDPRINT("\tavr data recieved on retry %d\n", avr_retry_count);
                    ctimer_stop(&avr_timeout_timer);
                    protobuf_data_t *pbd;
                    pbd = data;
                    sample.has_AVR = 1;
                    sample.AVR.size = pbd->length;
                    static uint8_t k;
                    for(k=0; k < pbd->length; k++){
                        sample.AVR.bytes[k] = pbd->data[k];
                    }
#ifdef AVRDEFBUG
                    SPRINT("\tRecieved %d bytes\t", pbd->length);
                    static uint8_t j;
                    for(j=0; j<pbd->length;j++){
                        SPRINT("%d:", pbd->data[j]);
                    }
                    SPRINT("\n");
#endif
//process data
                    if (avr_id < 0x10){
                        //it's a temp accel chain and so needs to be read twice to get valid data
                        //otherwise just once is ok
                        // It will break out of this loop when avr_recieved == 2
                        avr_recieved++;
                    }else{
                        avr_recieved = 2;
                    }
                }else{
                    AVRDPRINT("AVR timedout\n");
                    avr_retry_count++;
                }
                AVRDPRINT("avr_recieved = %d\n", avr_recieved);
            }while(avr_recieved < 2 && avr_retry_count < PROTOBUF_RETRIES);
        }
#ifndef SENSE_ON
        ms1_sense_off();
#endif

        ostream = pb_ostream_from_buffer(pb_buf, sizeof(pb_buf));
        pb_encode_delimited(&ostream, Sample_fields, &sample);

#ifndef SAMPLE_SEND
        filenames_next_write(filename);
        if(filename == 0) {
          continue;
        }

        AVRDPRINT("[SAMP] Writing %d bytes to %s...\n", ostream.bytes_written, filename);
    #ifdef SPI_LOCKING
        LPRINT("LOCK: write sample\n");
        NETSTACK_MAC.off(0);
        cc1120_arch_interrupt_disable();
        CC1120_LOCK_SPI();
    #endif
        fd = cfs_open(filename, CFS_WRITE | CFS_APPEND);
        if(fd >= 0){
            SPRINT("  [1/3] Writing to file...\n");
            cfs_write(fd, pb_buf, ostream.bytes_written);
            SPRINT("  [2/3] Closing file...\n");
            cfs_close(fd);
            SPRINT("  [3/3] Done\n");
        }else{
            SPRINT("[SAMP] Failed to open file %s\n", filename);
        }
    #ifdef SPI_LOCKING
        LPRINT("UNLOCK: write sample\n");
        CC1120_RELEASE_SPI();
        cc1120_arch_interrupt_enable();
        NETSTACK_MAC.on();
    #endif
#endif

    }
    PROCESS_END();
}

void 
avr_timer_handler(void *p)
{
	process_post(&sample_process, protobuf_event, (process_data_t)NULL);
}


void
refreshPosterConfig()
{
    if(get_config(&POST_config, COMMS_CONFIG) == 1){ 
        // Config file does not exist! Use default and set file
        POST_config.interval = POST_INTERVAL;
        POST_config.ip_count = POST_IP_COUNT;
        POST_config.ip[0] = POST_IP0;
        POST_config.ip[1] = POST_IP1;
        POST_config.ip[2] = POST_IP2;
        POST_config.ip[3] = POST_IP3;
        POST_config.ip[4] = POST_IP4;
        POST_config.ip[5] = POST_IP5;
        POST_config.ip[6] = POST_IP6;
        POST_config.ip[7] = POST_IP7;
        POST_config.port = POST_PORT;
        set_config(&POST_config, COMMS_CONFIG);
        //PPRINT("POST config set to default and written\n");
    }else{
        //PPRINT("POST Config file loaded\n");
    }
    //PPRINT("Refeshed post config to:\n");
    print_comms_config(&POST_config);

}
