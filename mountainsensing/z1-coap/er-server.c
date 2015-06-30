#include "er-server.h"
#include "rest-engine.h"
#include "contiki-net.h"

/* declare the resources functions from the separate files */
extern resource_t res_date, res_sample, res_config, res_reboot;

PROCESS(er_server_process, "CoAP Server");

PROCESS_THREAD(er_server_process, ev, data) {
    PROCESS_BEGIN();

    PROCESS_PAUSE();

    /* Initialize the REST engine. */
    rest_init_engine();

    rest_activate_resource(&res_date, "date");
    rest_activate_resource(&res_sample, "sample");
    rest_activate_resource(&res_config, "config");
    rest_activate_resource(&res_reboot, "reboot");

    while (1) {
        PROCESS_WAIT_EVENT();
    }                             /* while (1) */

    PROCESS_END();
}
