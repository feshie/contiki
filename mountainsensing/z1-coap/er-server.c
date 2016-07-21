#include "er-server.h"
#include "rest-engine.h"
#include "contiki-net.h"

/* declare the resources functions from the separate files */
extern resource_t res_date, res_sample, res_config, res_reboot, res_routes, res_uptime;

void er_server_init(void) {

    /* Initialize the REST engine. */
    rest_init_engine();

    rest_activate_resource(&res_date, "date");
    rest_activate_resource(&res_sample, "sample");
    rest_activate_resource(&res_config, "config");
    rest_activate_resource(&res_reboot, "reboot");
    rest_activate_resource(&res_routes, "routes");
    rest_activate_resource(&res_uptime, "uptime");
}
