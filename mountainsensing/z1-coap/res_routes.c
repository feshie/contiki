/**
 * @file
 * Route resource.
 * Displays the neighbour and route tables of the node.
 * Arthur Fabre
 */

#include "er-server.h"
#include "rest-engine.h"
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/rpl/rpl.h"

#define DEBUG_ON
#include "debug.h"

/**
 * Maximum length of an single nbr / route entry,
 * including terminating null byte
 */
#define MAX_ENTRY_SIZE 12

/**
 * The next route to use.
 * Can be null.
 */
static uip_ds6_route_t *route;

/**
 * The next neighbour to use.
 * Can be null.
 */
static uip_ds6_nbr_t *nbr;

/**
 * True if all the neighbors have been sent, and we should start sending routes.
 */
static bool hasReachedRoutes;

/**
 * Get last 2 bytes of an IP as a uint16_t that can be hex printed.
 */
static uint16_t get_hex_ip(uip_ipaddr_t *ip);

/**
 * Reset our internal state so that subsequent calls to get_next_nbr / route return the first ones.
 */
static void reset_nbr_route();

/**
 * Get the next neighbour if there is one, otherwise the next route. Set hasReachedRoutes accordingly.
 * @return true if something was received, false otherwise.
 */
static bool get_next_nbr_route();

/**
 * Get the next neighbour if there is one.
 * @return true if something was received, false otherwise.
 */
static bool get_next_nbr();

/**
 * Get the next route if there is one.
 * @return true if something was received, false otherwise.
 */
static bool get_next_route();

/**
 * Get handler for this resource
 */
static void res_get_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/**
 * Route resource.
 */
RESOURCE(res_routes, "Routes", res_get_handler, NULL, NULL, NULL);

void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
    int buffer_len = 0;

    DEBUG("Serving routes req Offset %d, PrefSize %d\n", (int) *offset, preferred_size);

    // If this is the first request get the first ip address
    if (*offset == 0) {
        reset_nbr_route();
        DEBUG("First request, resetting neighbour and route\n");
        if (!get_next_nbr_route()) {
            // No routes at all, end of request.
            char *error = "No nbrs / routes\n";
            DEBUG("%s", error);
            memcpy(buffer, error, strlen(error));
            REST.set_response_payload(response, buffer, strlen(error));
            // offset -1 means it is the end of a block req
            *offset = -1;
            return;
        }

        DEBUG("Printing preferred parent\n");

        rpl_dag_t *dag = rpl_get_any_dag();

        buffer_len += snprintf((char *) buffer, preferred_size, "%04x\n\n", get_hex_ip(rpl_get_parent_ipaddr(dag->preferred_parent)));
    }

    // Assume we have things left to send until we don't
    *offset += preferred_size;

    // Keep going until we don't have enough room to fit an entry in
    while (preferred_size - buffer_len > MAX_ENTRY_SIZE) {

        // Write a maximum of MAX_ENTRY_SIZE, as we know we have at least that much buffer space avail at this point.
        if (!hasReachedRoutes) {
            DEBUG("Printing nbr\n");
            buffer_len += snprintf((char *) buffer + buffer_len, MAX_ENTRY_SIZE, "%04x\n", get_hex_ip(&(nbr->ipaddr)));
        } else {
            DEBUG("Printing route\n");
            buffer_len += snprintf((char *) buffer + buffer_len, MAX_ENTRY_SIZE, "%04x@%04x\n", get_hex_ip(&(route->ipaddr)), get_hex_ip(uip_ds6_route_nexthop(route)));
        }

        // Get the next thing
        if (!get_next_nbr_route()) {
            // No next route, end of request.
            DEBUG("No more routes\n");
            *offset = -1;
            break;
        }
    }

    REST.set_response_payload(response, buffer, buffer_len);
}

inline uint16_t get_hex_ip(uip_ipaddr_t *ipaddr) {
    return (ipaddr->u8[14] << 8) + ipaddr->u8[15];
}

inline void reset_nbr_route() {
    route = NULL;
    nbr = NULL;
    hasReachedRoutes = false;
}

inline bool get_next_nbr_route() {
    // If we haven't reached the end of the neighbours, and there is still one more neighbour, get that.
    if (!hasReachedRoutes && !(hasReachedRoutes = !get_next_nbr())) {
        return true;
    }

    // Otherwise attempt to get a route
    return get_next_route();
}

inline bool get_next_nbr() {
    if (nbr == NULL) {
        nbr = nbr_table_head(ds6_neighbors);
    } else {
        nbr = nbr_table_next(ds6_neighbors, nbr);
    }

    DEBUG("Got nbr %p\n", nbr);
    return nbr != NULL;
}

inline bool get_next_route() {
    if (route == NULL) {
        route = uip_ds6_route_head();
    } else {
        route = uip_ds6_route_next(route);
    }

    DEBUG("Got route %p\n", route);
    return route != NULL;
}
