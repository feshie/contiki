#include "er-server.h"
#include "rest-engine.h"

#define DEBUG

#ifdef DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

extern resource_t res_hello;
/* declare the resources functions from the separate files *

/**
 * Ressources for config (GET and POST)
 */
//RESOURCE(config_get, METHOD_GET, "config", "");
//RESOURCE(config_post, METHOD_POST, "config", "");

//void config_get_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {

//}

//void config_post_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {

//}

/**
 * Test HELLO ressource
 */

/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
void res_get_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
    char const * const message = "Hello World!";
    int length = 12;

    memcpy(buffer, message, length);

    REST.set_header_content_type(response, REST.type.TEXT_PLAIN); 
    REST.set_header_etag(response, (uint8_t *) &length, 1);
    REST.set_response_payload(response, buffer, length);
}

RESOURCE(res_hello, "title=\"Hello world: ?len=0..\";rt=\"Text\"", res_get_handler, NULL, NULL, NULL);
 */

PROCESS(er_server_process, "CoAP Server");
PROCESS_THREAD(er_server_process, ev, data) {
    PROCESS_BEGIN();

    PROCESS_PAUSE();

    /* Initialize the REST engine. */
    rest_init_engine();

    rest_activate_resource(&res_hello, "hello");

    //rest_activate_resource(&res_wais, "");
    //wais_activate_sensors();

    while (1) {
        PROCESS_WAIT_EVENT();
    }                             /* while (1) */

    PROCESS_END();
}




/*
static
PT_THREAD(web_handle_connection(struct psock *p))
{

    static uint8_t i;
    static char num[16], tmpstr[120];
    static uint16_t y;
    static uint8_t mo, d, h, mi, se;
    static bool submitted;
    static const char* ZERO = "0";
    static uint8_t clock_ret;
    static char AVRs[32];
    static SensorConfig sensor_config;
    static POSTConfig POST_config;
    static char *url;
    static char param[URL_PARAM_LENGTH];
    static uint32_t bytesused;
    static int filecount;
    static char *pch;
    static char chr[2] = {'a',0};
    static struct cfs_dir dir;
    static struct cfs_dirent dirent;

    WPRINT("[WEBD] Reading HTTP request line...\n");
    PSOCK_BEGIN(p);
    PSOCK_READTO(p, '\n');
    if(strncmp("GET ", (char*)web_buf, 4) == 0){
        url = web_buf + 4;
        strtok(url, " ");
        WPRINT("[WEBD] Got request for %s\n", url);

        if(strncmp(url, "/clock", 6) == 0){
            // Serve clock form
            WPRINT("[WEBD] Serving /clock\n");
            submitted = 0;
            if(get_url_param(param, url, "submit") == 1) {
                WPRINT("[WEBD] Clock submitted\n");
                submitted = 1;
                get_url_param(param, url, "y");
                y = atol(param == NULL ? ZERO : param);
                get_url_param(param, url, "mo");
                mo = atoi(param == NULL ? ZERO : param);
                get_url_param(param, url, "d");
                d = atoi(param == NULL ? ZERO : param);
                get_url_param(param, url, "h");
                h = atoi(param == NULL ? ZERO : param);
                get_url_param(param, url, "mi");
                mi = atoi(param == NULL ? ZERO : param);
                get_url_param(param, url, "s");
                se = atoi(param == NULL? ZERO : param);
                clock_ret = set_time(y, mo, d, h, mi, se);
                WPRINT("Clock = %lu\n", get_time());
            }
            PSOCK_SEND_STR(p, HTTP_RES);
            PSOCK_SEND_STR(p, TOP);
            if(submitted) {
                if(clock_ret == 0){
                    sprintf(tmpstr, "<h1>Success! Time set to %lu</h1>", get_time());
                    PSOCK_SEND_STR(p, tmpstr);
                }else{
                    PSOCK_SEND_STR(p, "<h1>Warning, set time returned non-zero status</h1>");
                }
            }else{
                PSOCK_SEND_STR(p, CLOCK_FORM);
            }

            PSOCK_SEND_STR(p, BOTTOM);
        }else if(strncmp(url, "/sample", 7) == 0){
            PSOCK_SEND_STR(p, HTTP_RES);
            PSOCK_SEND_STR(p, TOP);
            PSOCK_SEND_STR(p, SENSOR_FORM_1);
            //Interval
            ltoa(sensor_config.interval, tmpstr, 10);
            PSOCK_SEND_STR(p, tmpstr);
            PSOCK_SEND_STR(p, SENSOR_FORM_2);
            //AVR IDs
            WPRINT("[WEBD] Producing AVR IDs from config\n");
            for(i = 0; i < sensor_config.avrIDs_count; i++) {
                WPRINT(".");
                itoa(sensor_config.avrIDs[i], tmpstr, 10);
                PSOCK_SEND_STR(p, tmpstr);
                if(i != sensor_config.avrIDs_count - 1) {
                    PSOCK_SEND_STR(p, ".");
                }
            }
            WPRINT("\n[WEBD] Producing other data\n");
            PSOCK_SEND_STR(p, SENSOR_FORM_3);
            //Rain?
            if(sensor_config.hasRain) {
                PSOCK_SEND_STR(p, " checked");
            }
            WPRINT("  RAIN DONE\n");
            PSOCK_SEND_STR(p, SENSOR_FORM_4);
            //ADC1?
            if(sensor_config.hasADC1) {
                PSOCK_SEND_STR(p, " checked");
            }
            WPRINT("  ADC1 DONE\n");
            PSOCK_SEND_STR(p, SENSOR_FORM_5);
            //ADC2?
            if(sensor_config.hasADC2) {
                PSOCK_SEND_STR(p, " checked");
            }
            WPRINT("  ADC2 DONE\n");
            PSOCK_SEND_STR(p, SENSOR_FORM_6);
            PSOCK_SEND_STR(p, BOTTOM);
            WPRINT("[WEBD] Closing connection.\n");
        }else if(strncmp(url, "/sensub", 7) == 0){
            if(get_url_param(param, url, "sample") == 1){
                sensor_config.interval = atol(param);
            }else{
                //no value specified using default
                sensor_config.interval = SENSOR_INTERVAL;
            }

            if(get_url_param(param, url, "AVR") ==1){
                strcpy(AVRs, param);
                pch = strtok(AVRs, ".");
                i = 0;
                while(pch != NULL) {
                    sensor_config.avrIDs[i++] = strtol(pch, NULL, 16);
                    pch = strtok(NULL, ".");
                }
                sensor_config.avrIDs_count = i;
            }

            if(get_url_param(param, url, "rain") == 1 && strcmp(param, "y") == 0) {
                sensor_config.hasRain = 1;
            } else {
                sensor_config.hasRain = 0;
            }

            if(get_url_param(param, url, "adc1") == 1 && strcmp(param, "y") == 0) {
                sensor_config.hasADC1 = 1;
            } else {
                sensor_config.hasADC1 = 0;
            }


            if(get_url_param(param, url, "adc2") == 1 && strcmp(param, "y") == 0) {
                sensor_config.hasADC2 = 1;
            } else {
                sensor_config.hasADC2 = 0;
            }

            set_config(&sensor_config, SAMPLE_CONFIG);
            refreshSensorConfig();

            WPRINT("[WEBD] Stored Data\n");
            PSOCK_SEND_STR(p, HTTP_RES);
            PSOCK_SEND_STR(p, TOP);
            PSOCK_SEND_STR(p, "<h1>OK</h1>");
            PSOCK_SEND_STR(p, BOTTOM);
        }else if(strncmp(url, "/comms", 6) == 0){
            WPRINT("Comms form\n");
            //should put a lot of text into a string and send rather than like this
            PSOCK_SEND_STR(p, HTTP_RES);
            PSOCK_SEND_STR(p, TOP);
            PSOCK_SEND_STR(p, COMMS_FORM_1);
            // Interval
            ltoa(POST_config.interval, tmpstr, 10);
            PSOCK_SEND_STR(p, tmpstr);
            PSOCK_SEND_STR(p, COMMS_FORM_2A);
            ltoa(POST_config.ip[0], tmpstr, 16);
            PSOCK_SEND_STR(p, tmpstr);
            PSOCK_SEND_STR(p, COMMS_FORM_2B);
            ltoa(POST_config.ip[1], tmpstr, 16);
            PSOCK_SEND_STR(p, tmpstr);
            PSOCK_SEND_STR(p, COMMS_FORM_2C);
            ltoa(POST_config.ip[2], tmpstr, 16);
            PSOCK_SEND_STR(p, tmpstr);
            PSOCK_SEND_STR(p, COMMS_FORM_2D);
            ltoa(POST_config.ip[3], tmpstr, 16);
            PSOCK_SEND_STR(p, tmpstr);
            PSOCK_SEND_STR(p, COMMS_FORM_2E);
            ltoa(POST_config.ip[4], tmpstr, 16);
            PSOCK_SEND_STR(p, tmpstr);
            PSOCK_SEND_STR(p, COMMS_FORM_2F);
            ltoa(POST_config.ip[5], tmpstr, 16);
            PSOCK_SEND_STR(p, tmpstr);
            PSOCK_SEND_STR(p, COMMS_FORM_2G);
            ltoa(POST_config.ip[6], tmpstr, 16);
            PSOCK_SEND_STR(p, tmpstr);
            PSOCK_SEND_STR(p, COMMS_FORM_2H);
            ltoa(POST_config.ip[7], tmpstr, 16);
            PSOCK_SEND_STR(p, tmpstr);

            PSOCK_SEND_STR(p, COMMS_FORM_3);

            ltoa(POST_config.port, tmpstr, 10);
            PSOCK_SEND_STR(p, tmpstr);
            PSOCK_SEND_STR(p, COMMS_FORM_4);

            PSOCK_SEND_STR(p, BOTTOM);
        }else if(strncmp(url, "/comsub", 7) == 0){
            WPRINT("Comms settings being set\n");
            if(get_url_param(param, url, "interval") == 1){
                POST_config.interval = atol(param);
                WPRINT("Interval set to %d\n", (uint16_t)POST_config.interval);
            }else{
                POST_config.interval = POST_INTERVAL;
                WPRINT("Interval not submitted\n");
            }


            WPRINT("Setting IP to:");
            for(i = 0; i < 8; i++) {
                chr[0] = 'a' + i;
                if (get_url_param(param, url, chr) ==1){
                    POST_config.ip[i] = strtol(param, NULL, 16);
                    WPRINT("%x",(unsigned int)POST_config.ip[i]);
                    if (i < 7){
                        WPRINT(":");
                    }else{
                        WPRINT("\n");
                    }
                }else{
                    printf("incomplete IP given. ABORTING\n");
                    goto close_connection;
                }
            }

            if(get_url_param(param, url, "port") == 1){
                POST_config.port = atol(param);
                WPRINT("Setting port to %d\n",(unsigned int)POST_config.port );
            }else{
                POST_config.port = POST_PORT;
                WPRINT("Port not submitted\n");
            }
            if(set_config((void *)&POST_config, COMMS_CONFIG) == 0){
                refreshPosterConfig();
                PSOCK_SEND_STR(p, HTTP_RES);
                strcpy(tmpstr,TOP);
                strcat(tmpstr, "<h1>OK</h1>");
                strcat(tmpstr, BOTTOM);
                PSOCK_SEND_STR(p, tmpstr);
            }else{
                printf("Setting config failed\n");
                PSOCK_SEND_STR(p, HTTP_RES);
                strcpy(tmpstr,TOP);
                strcat(tmpstr, "<h1>FAILED</h1>");
                strcat(tmpstr, BOTTOM);
                PSOCK_SEND_STR(p, tmpstr);
            }
        }else if(strncmp(url, "/du", 3) == 0){
            // debug call to see flash disk usage
            if( flash_du(&filecount, &bytesused) == -1){
                PSOCK_SEND_STR(p, "failed\n");
                goto close_connection;
            }

            sprintf(tmpstr, "%s\n%d files %ld bytes\n",TEXT_RES,filecount,bytesused);
            PSOCK_SEND_STR(p, tmpstr);
        }else if(strncmp(url, "/ls", 3) == 0){
            // debug call to list all files on flash = SLOW!

            PSOCK_SEND_STR(p, TEXT_RES);
            PSOCK_SEND_STR(p, "printing on serial\r\n");
            if(cfs_opendir(&dir, "/") == 0) {
                while(cfs_readdir(&dir, &dirent) != -1) {
                    printf("%s %ld\n", dirent.name, (long)dirent.size);
                }
            }
        }else if(strncmp(url, "/settings", 9) == 0){
            // debug GET for the node settings gives sampleinterval adc1 adc2 rain
            PSOCK_SEND_STR(p, HTTP_RES);
            PSOCK_SEND_STR(p, TOP);
            // get time
            ltoa(get_time(), tmpstr, 10);
            strcat(tmpstr, " ");
            // sample Interval
            ltoa(sensor_config.interval, num, 10);
            strcat(tmpstr, num);
            strcat(tmpstr, "s ");
            // POST Interval
            ltoa(POST_config.interval, num, 10);
            strcat(tmpstr, num);
            strcat(tmpstr, "P ");
            if( sensor_config.hasADC1 == 1){
                strcat(tmpstr, "A1 ");
            }
            if( sensor_config.hasADC2 == 1){
                strcat(tmpstr, "A2 ");
            }
            if( sensor_config.hasRain == 1){
                strcat(tmpstr, "R ");
            }
            ltoa(reset_sensor.value(0), num, 10);
            strcat(tmpstr, num);
            PSOCK_SEND_STR(p, tmpstr);
            PSOCK_SEND_STR(p, BOTTOM);
        }else if(strncmp(url, "/json", 5) == 0){
            WPRINT("Serving JSON file\n");
            PSOCK_SEND_STR(p, JSON_RES);
            sprintf(tmpstr, "{\"reading\":{\"timestamp\":%lu,\"temperature\":%d.%03u,\"battery\":%u.%03u,\"x\":%d,\"y\":%d,\"z\":%d",
                get_time(),
                (int)get_sensor_temp(),
                (unsigned)((get_sensor_temp() - (int)get_sensor_temp())*1000),
                (int)get_sensor_batt(),
                (unsigned)((get_sensor_batt() - (int)get_sensor_batt())*1000),
                get_sensor_acc_x(),
                get_sensor_acc_y(),
                get_sensor_acc_z());

            if(get_url_param(param, url, "adc1") == 1 && strcmp(param, "y") == 0) {
                sprintf(tmpstr + strlen(tmpstr), ",\"adc1\":%d", get_sensor_ADC1());
            }
            if(get_url_param(param, url, "adc2") == 1 && strcmp(param, "y") == 0) {
                sprintf(tmpstr + strlen(tmpstr), ",\"adc2\":%d", get_sensor_ADC2());
            }
            strcat(tmpstr, "}}");
            PSOCK_SEND_STR(p, tmpstr);
        }else{
            WPRINT("Serving / \"INDEX\"\n");
            PSOCK_SEND_STR(p, HTTP_RES);
            PSOCK_SEND_STR(p, TOP);
            PSOCK_SEND_STR(p, INDEX_BODY);
            PSOCK_SEND_STR(p, BOTTOM);
        }
    }
    close_connection:
    PSOCK_CLOSE(p);
    PSOCK_END(p);
}

PROCESS_THREAD(web_process, ev, data)
{

    PROCESS_BEGIN();

    tcp_listen(UIP_HTONS(80));
    while(1){
        WPRINT("[WEBD] Now listening for connections\n");
        PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
        if(uip_connected()){
            WPRINT("[WEBD] Connected!\n");
            PSOCK_INIT(&web_ps, (uint8_t *)web_buf, sizeof(web_buf));
            while(!(uip_aborted() || uip_closed() || uip_timedout())){
                WPRINT("[WEBD] Waiting for TCP Event\n");
                PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
                WPRINT("[WEBD] Handle connection\n");
                web_handle_connection(&web_ps);
                WPRINT("[WEBD] Handle COMPLETE!\n");
            }
        }
    }
    PROCESS_END();
}*/

/*
 * Gets the value associated with the key in a URL
 *
 * Example:
 *   url = "http://ecs.soton.ac.uk?name=djap1g11&login=false"
 *   key = "name"
 *
 *   Returns "djap1g11"
 */
/*uint8_t
get_url_param(char *par, char *url, char *key)
{
    char str[URL_LENGTH];
    char *pch;
    uint8_t len;
    uint8_t ret_status;
    strcpy(str, url);
  //  WPRINT("looking for %s, in %s\n", key, url);
    len = strlen(key);
  //  WPRINT("key length = %d\n", len);
    pch = strtok(str, "?&");
    ret_status =  0;
    while(pch != NULL) {
        if(strncmp(pch, key, len) == 0) {
            // If the token is key-value pair desired
            pch = pch + len + 1;
            strcpy(par, pch);
            ret_status =  1;
            break;
        }
        pch = strtok(NULL, "?&");
    }
  //  WPRINT("Found %s\n", par);

    return ret_status;
}*/




/* returns the number of files and disk used on Flash
*/
/*int
flash_du(int *filec, uint32_t *bytes)
{
    struct cfs_dir dir;
    struct cfs_dirent dirent;
    static int count = 0;
    static uint32_t used = 0;

    if(cfs_opendir(&dir, "/") == 0) {
        while(cfs_readdir(&dir, &dirent) != -1) {
            count++;
           used += dirent.size;
        }
        *bytes = used;
        *filec = count;
        //cfs_closedir(&dir);
        return(0);
    }else {
        return(-1);
    }

    //cfs_closedir(&dirp);
    *bytes = used;
    *filec = count;
    return(0);
}*/
