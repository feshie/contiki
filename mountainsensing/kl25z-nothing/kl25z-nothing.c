/**
 * \file
 *  Dummy contiki application that des nothing. Literally.
 *  Might be useful for relay only nodes.
 */
#include <stdbool.h>
#include "contiki.h"

PROCESS(do_nothing_process, "Process that does nothing");

AUTOSTART_PROCESSES(&do_nothing_process);

PROCESS_THREAD(do_nothing_process, ev, data) {
    PROCESS_BEGIN();

    while (true) {
        PROCESS_YIELD();
    }

    PROCESS_END();
}
