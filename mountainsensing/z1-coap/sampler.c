#include "sampler.h"

PROCESS(sample_process, "Sample Process");

#define SENSEDEFBUG
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

/**
 * Print the sensor config.
 */
void print_sensor_config(SensorConfig *conf);

static SensorConfig sensor_config;
static process_event_t protobuf_event;

void refreshSensorConfig(void) {
    if (!store_get_config(&sensor_config)) {
        // Config file does not exist! Use default and set file
      	SPRINT("No Sensor config found\n");
        sensor_config.interval = SENSOR_INTERVAL;
        sensor_config.avrIDs_count = SENSOR_AVRIDS_COUNT;
        sensor_config.hasADC1 = SENSOR_HASADC1;
        sensor_config.hasADC2 = SENSOR_HASADC2;
        sensor_config.hasRain = SENSOR_HASRAIN;
        store_save_config(&sensor_config);
    } else {
      	SPRINT("Sensor config loaded\n");
    }
}

PROCESS_THREAD(sample_process, ev, data) {
    static struct etimer sample_timer;
    static Sample sample;
    static int i;
    static uint8_t avr_id;
    static struct ctimer avr_timeout_timer;
    static uint8_t avr_recieved;
    static uint8_t avr_retry_count;
    static int16_t id;

    PROCESS_BEGIN();

    refreshSensorConfig();
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

        id = store_save_sample(&sample);

        if (id < 0) {
            SPRINT("[SAMP] Failed to save sample!\n");
        } else {
            SPRINT("[SAMP] Sample saved with id %d\n", id);
        }
    }
    PROCESS_END();
}

void print_sensor_config(SensorConfig *conf) {
    static uint8_t i;
    SPRINT("\tInterval = %d\n", (unsigned int)conf->interval);
    SPRINT("\tADC1: ");
    if (conf->hasADC1 == 1){
        SPRINT("yes\n");
    } else {
        SPRINT("no\n");
    }
    SPRINT("\tADC2: ");
    if (conf->hasADC2) {
        SPRINT("yes\n");
    } else {
        SPRINT("no\n");
    }
    SPRINT("\tRain: ");
    if (conf->hasRain) {
        SPRINT("yes\n");
    } else {
        SPRINT("no\n");
    }
    SPRINT("\tAVRs: ");
    if (conf->avrIDs_count == 0) {
        SPRINT("NONE\n");
    } else {
        for(i = 0; i < conf->avrIDs_count; i++) {
            SPRINT("%02x", (int)conf->avrIDs[i] & 0xFF);
            if (i < conf->avrIDs_count - 1) {
                SPRINT(", ");
            } else {
                SPRINT("\n");
            }
        }
    }
}

void avr_timer_handler(void *p) {
	process_post(&sample_process, protobuf_event, (process_data_t)NULL);
}
