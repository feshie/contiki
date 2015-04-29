
#include "sampler.h"
#include "poster.h"

PROCESS(sample_process, "Sample Process");

#define SENSEDEFBUG
#ifdef SENSEDEFBUG
    #define SPRINT(...) printf(__VA_ARGS__)
#else
    #define SPRINT(...)
#endif

//#define AVRDEFBUG
#ifdef AVRDEFBUG
    #define AVRDPRINT(...) printf(__VA_ARGS__)
#else
    #define AVRDPRINT(...)
#endif

//#define SENSE_ON /*Do not turn sensor power off */

static SensorConfig sensor_config;
static POSTConfig POST_config;
static process_event_t protobuf_event;
static struct psock ps;
static uint8_t psock_buffer[PSOCK_BUFFER_LENGTH];


void
refreshSensorConfig(void){
    if(get_config(&sensor_config, SAMPLE_CONFIG) == 1){ 
        // Config file does not exist! Use default and set file
      	printf("No Sensor config found\n");
        sensor_config.interval = SENSOR_INTERVAL;
        sensor_config.avrIDs_count = SENSOR_AVRIDS_COUNT;
        sensor_config.hasADC1 = SENSOR_HASADC1;
        sensor_config.hasADC2 = SENSOR_HASADC2;
        sensor_config.hasRain = SENSOR_HASRAIN;
        set_config(&sensor_config, SAMPLE_CONFIG);
    }else{
      	printf("Sensor config loaded\n");
    }
}






PROCESS_THREAD(sample_process, ev, data)
{
    //sample variables
    static char filename[FILENAME_LENGTH];

    //poster variables
    static uint8_t post_retries;
    static uip_ipaddr_t addr;
    static char data_buffer[DATA_BUFFER_LENGTH];
    static uint8_t http_status;
    static uint8_t data_length;
    static struct etimer timeout_timer;
    static struct etimer assoc_timer;
    static struct etimer delay_timer;
    static uip_ds6_addr_t *n_addr;



    PROCESS_BEGIN();
    filenames_init();
    data_length = 0;
    http_status = 0;

    protobuf_event = process_alloc_event();
    protobuf_register_process_callback(&sample_process, protobuf_event) ;
    printf("!!!!!!!!!!Z1 DATA DUMPER!!!!!!!!!!!!!!!\n");
    data_length = 0;
    http_status = 0;
    
    do{
        n_addr = uip_ds6_get_global(-1);
        printf("Not associated so can't send\n");
        
        etimer_set(&assoc_timer, CLOCK_SECOND * 5);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&assoc_timer));
    }while(n_addr == NULL);
    printf("Now associated\n");
    while((filenames_next_read(filename)) !=0){
        data_length = load_file(data_buffer, filename);

        uip_ip6addr_u8(&addr,
            n_addr->ipaddr.u8[0], n_addr->ipaddr.u8[1],
            n_addr->ipaddr.u8[2], n_addr->ipaddr.u8[3],
            n_addr->ipaddr.u8[4], n_addr->ipaddr.u8[5],
            n_addr->ipaddr.u8[6], n_addr->ipaddr.u8[7],
            0, 0, 0, 0, 0, 0, 0, 1);
        printf("About to post to: ");
        uip_debug_ipaddr_print(&addr);
        printf("\n");
        PPRINT("[POST][INIT] About to attempt POST with %s - RETRY [%d]\n", filename, post_retries);
        PPRINT("Data length = %d\n", data_length);
        if(data_length == 0 && strcmp("r0", filename) == 0){                    
            printf("Enpty file r0 breaking out of send loop\n");
            break;
        }else if (data_length ==0){
            //something odd has happened
            PPRINT("Length = 0\n");
            filenames_refresh();
            continue;
        }
        tcp_connect(&addr, UIP_HTONS(8081), NULL);
        PPRINT("Connecting...");
        PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
        if(uip_aborted() || uip_timedout() || uip_closed() ) {
            PPRINT("Could not establish connection\n");
            printf("UIP flags = %d\n", uip_flags);
            if(uip_flags == UIP_ABORT){
                PPRINT("Connection Aborted\n");
            }else if(uip_flags == UIP_TIMEDOUT){
                PPRINT("UIP Timeout\n");
            }else if(uip_flags == UIP_CLOSE){
                PPRINT("UIP Closed\n");
            }
            post_retries++;
        } else if(uip_connected() || uip_poll()) {
            PPRINT("Connected\n");
            PSOCK_INIT(&ps, psock_buffer, sizeof(psock_buffer));
            etimer_set(&timeout_timer, CLOCK_SECOND*LIVE_CONNECTION_TIMEOUT);
            do {
                if(etimer_expired(&timeout_timer)){
                    PPRINT("Connection took too long. TIMEOUT\n");
                    PSOCK_CLOSE(&ps);
                    post_retries++;
                    break;
                }else if(http_status == 0){
                    PPRINT("[POST] Handle Connection\n");
                    handle_connection(data_buffer, data_length, &http_status, &ps, &psock_buffer);
                    //not returned yet
                    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
                }else{
                    PPRINT("HTTP status = %d\n", http_status);
                    break;
                }
            } while(!(uip_closed() || uip_aborted() || uip_timedout()));
            if(uip_flags == UIP_ABORT){
                PPRINT("Connection Aborted\n");
            }else if(uip_flags == UIP_TIMEDOUT){
                PPRINT("UIP Timeout\n");
            }else if(uip_flags == UIP_CLOSE){
                PPRINT("UIP Closed\n");
            }else if(http_status ==0){
                PPRINT("!!!!!!Other flags not sure why I exited loop: %d\n", uip_flags);
            }
            PPRINT("Buffer = %s\n",psock_buffer);
            if(http_status == 0 && strncmp(psock_buffer, "HTTP/", 5) == 0){   
                // Status line
                http_status = atoi((const char *)psock_buffer + 9);
                PPRINT("Setting status outside loop\n");
            }
           // PPRINT("Connection closed.\n");
            PPRINT("HTTP Status = %d\n", http_status);
            if(http_status/100 == 2) { // Status OK
                data_length = 0;
                post_retries = 0;
                http_status = 0;
                filenames_delete(filename);
                PPRINT("[POST] Removing file\n");
            } else { // POST failed
                PPRINT("[POST] Failed, not removing file\n");
                data_length = 0;
                post_retries++;
                http_status = 0;    
            }
        }else{
            printf("Other status\n");
            printf("UIP flags = %d\n", uip_flags);
        }
        memcpy(psock_buffer, '0', strlen(psock_buffer));
        PSOCK_CLOSE(&ps);
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
        PPRINT("POST config set to default and written\n");
    }else{
        PPRINT("POST Config file loaded\n");
    }
    PPRINT("Refeshed post config to:\n");
    print_comms_config(&POST_config);

}
