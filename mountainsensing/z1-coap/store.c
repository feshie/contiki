#include "store.h"
#include "stdio.h"
#include "stdlib.h"
#include "contiki.h"
#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"
#include "pb_decode.h"
#include "pb_encode.h"

#ifdef SPI_LOCKING
    #include "dev/cc1120.h"
    #include "dev/cc1120-arch.h"
#endif

#define DEBUG_ON
#include "debug.h"

PROCESS(store_process, "Store Process");

/**
 * @file
 * The store runs in a seperate process, and waits for events.
 * The store can only handle one event at a time, ensuring there are no race conditions.
 *
 * The events are dispatched by the convenience functions decalred in store.h
 *
 * To avoid memory shenanigans, all the memory used to return or accept values from other threads
 * must be allocated by the caller.
 *
 * The store allows deleting any given sample, by gracefully dealing with files that do not exist.
 */


/**
 * Single char prefix to use for filenames
 */
#define FILENAME_PREFIX "r"

/**
 * Filename of the config file
 */
#define CONFIG_FILENAME "sampleconfig"

/*
 * Size is the largest one of SensorConfig and Sample
 */
#if Sample_size > SensorConfig_size
    #define PB_BUF_SIZE Sample_size
#else
    #define PB_BUF_SIZE SensorConfig_size
#endif

/**
 * Maximum length of a file name
 */
#define FILENAME_LENGTH 8

/**
 * Magic canary to indicate that processing the given event has failed.
 * Everything will break if the start of a Config or Sample is every equal to this.
 */
#define STORE_PROCESS_FAIL      INT16_MAX

/**
 * Data should point to sample to save
 * Data will point to a int16_t >= 0 on success, STORE_PROCESS_FAIL otherwise.
 */
#define STORE_EVENT_SAVE_SAMPLE             1

/**
 * Data should point to a int16_t (but needs to point to a chunk of memory Sample sized)
 * Data will point to the sample with that id on success, STORE_PROCESS_FAIL otherwise.
 */
#define STORE_EVENT_GET_SAMPLE              2

/**
 * Data should point to a int16_t (but needs to point to a chunk of memory Sample sized)
 * Data will point to the proto buffer encoded sample with that id on success, STORE_PROCESS_FAIL otherwise.
 */
#define STORE_EVENT_GET_RAW_SAMPLE          3

/**
 * Data should point to a chunk of memory Sample sized.
 * Data will point to the lastest sample on success, STORE_PROCESS_FAIL otherwise.
 */
#define STORE_EVENT_GET_LATEST_SAMPLE       4

/**
 * Data should point to a chunk of memory Sample sized.
 * Data will point to the lastest sample iproto buffer encoded on success, STORE_PROCESS_FAIL otherwise.
 */
#define STORE_EVENT_GET_LATEST_RAW_SAMPLE   5

/**
 * Data should point to a int16_t
 * Data will point to STORE_PROCESS_FAIL on failure.
 */
#define STORE_EVENT_DELETE_SAMPLE           6

/**
 * Data should point the SampleConfig to save
 * Data will point to STORE_PROCESS_FAIL on failure.
 */
#define STORE_EVENT_SAVE_CONFIG             7

/**
 * Data should point to a chunk of memory SampleConfig sized
 * Data will point to the SampleConfig on success, STORE_PROCESS_FAIL otherwise.
 */
#define STORE_EVENT_GET_CONFIG              8

/**
 * Data should point to a chunk of memory SampleConfig sized
 * Data will point to the proto buff encoded SampleConfig on success, STORE_PROCESS_FAIL otherwise.
 */
#define STORE_EVENT_GET_RAW_CONFIG          9

// Stack variable paranoia.
// These are used by the event handlers.
static pb_ostream_t pb_ostream;
static pb_istream_t pb_istream;
static uint8_t pb_buffer[Sample_size];
static char filename[FILENAME_LENGTH];
static int fd;
static int bytes;

/**
 * Identifier of the last sample.
 */
static int16_t last_id;

/**
 * Initialize the data store.
 * Includes finding the latest reading.
 */
static void store_init(void);

/**
 * Lock the radio for cfs access.
 */
static void radio_lock(void);

/**
 * Release the radio after cfs access.
 */
static void radio_release(void);

/**
 * Find the id of the latest sample.
 */
static int16_t find_latest_sample(void);

/**
 * Write to a given file.
 * @return true if length bytes have been successfully written to filename, false otherwise.
 */
static bool write_file(char* filename, uint8_t *buffer, int length);

/**
 * Save a sample.
 * @return STORE_PROCESS_FAIL on failure, the id of the sample on success.
 */
static int16_t save_sample(Sample *sample);

/**
 * Get a Sample from flash.
 * @return true on success, false otherwise (including if the file does not exist)
 */
static bool get_sample(int16_t id, Sample *sample);

/**
 * Get a Sample from flash, in the form a raw protocol buffer.
 * @return true on success, false otherwise (including if the file does not exist)
 */
static bool get_raw_sample(int16_t id, uint8_t buffer[Sample_size]);

/**
 * Delete a given sample. Will search backwards for the last known stored Sample
 * if sample is the latest Sample.
 * @return true on success, false otherwise.
 */
static bool delete_sample(int16_t sample);

/**
 * Actually save the configuration.
 * @return true on success, false otherwise.
 */
static bool save_config(SensorConfig *config);

/**
 * Actually get the configuration.
 * @return true on success, false otherwise.
 */
static bool get_config(SensorConfig *config);

/**
 * Actually get the configuration, in the form of a raw protocol buffer.
 * @return true on success, false otherwise.
 */
static bool get_raw_config(uint8_t buffer[SensorConfig_size]);

/**
 * Convert a sample id to a filename.
 * @return The pointer to filename(usefull for avoiding temp vars).
 */
static char* id_to_file(int16_t id, char* filename);

PROCESS_THREAD(store_process, ev, data) {
    PROCESS_BEGIN();

    DEBUG("Initializing...\n");

    store_init();

    printf("Store started. %d previous files found.\n", last_id);

    while (true) {
        PROCESS_WAIT_EVENT();

        DEBUG("Got event %d\n", ev);

        if (ev == STORE_EVENT_SAVE_SAMPLE) {
            *((int16_t *) data) = save_sample((Sample *) data);

        } else if (ev == STORE_EVENT_GET_SAMPLE) {
            // Id is passed by value so that it doesn't change when something is written to the passed buffer
            if (!get_sample(*((int16_t *) data), (Sample *) data)) {
                *((int16_t *) data) = STORE_PROCESS_FAIL;
            }

        } else if (ev == STORE_EVENT_GET_RAW_SAMPLE) {
            // Id is passed by value so that it doesn't change when something is written to the passed buffer
            if (!get_raw_sample(*((int16_t *) data), (uint8_t *) data)) {
                *((int16_t *) data) = STORE_PROCESS_FAIL;
            }

        } else if (ev == STORE_EVENT_GET_LATEST_SAMPLE) {
            // Just get last_id. We keep our state clean (ie last_id always points to a valid Sample) so this isn't an issue.
            if (!get_sample(last_id, (Sample *) data)) {
                *((int16_t *) data) = STORE_PROCESS_FAIL;
            }

        } else if (ev == STORE_EVENT_GET_LATEST_RAW_SAMPLE) {
            // Just get last_id. We keep our state clean (ie last_id always points to a valid Sample) so this isn't an issue.
            if (!get_raw_sample(last_id, (uint8_t *) data)) {
                *((int16_t *) data) = STORE_PROCESS_FAIL;
            }

        } else if (ev == STORE_EVENT_DELETE_SAMPLE) {
            if (!delete_sample(*((int16_t *) data))) {
                *((int16_t *) data) = STORE_PROCESS_FAIL;
            }

        } else if (ev == STORE_EVENT_SAVE_CONFIG) {
            // Only overwrite the config if we failed
            if (!save_config((SensorConfig *) data)) {
                *((int16_t *) data) = STORE_PROCESS_FAIL;
            }

        } else if (ev == STORE_EVENT_GET_CONFIG) {
            // If succesfull, just return the buffer.
            // Otherwise set it to false
            if (!get_config((SensorConfig *) data)) {
                *((int16_t *) data) = STORE_PROCESS_FAIL;
            }

        } else if (ev == STORE_EVENT_GET_RAW_CONFIG) {
            // If succesfull, just return the buffer.
            // Otherwise set it to false
            if (!get_raw_config((uint8_t *) data)) {
                *((int16_t *) data) = STORE_PROCESS_FAIL;
            }

        } else {
            DEBUG("Unknown Event %d!\n", ev);
        }
    }

    PROCESS_END();
}

int16_t save_sample(Sample *sample) {

    last_id++;

    sample->id = last_id;

    DEBUG("Attempting to save reading with id %d\n", last_id);

    pb_ostream = pb_ostream_from_buffer(pb_buffer, sizeof(pb_buffer));
    if (!pb_encode_delimited(&pb_ostream, Sample_fields, sample)) {
        last_id--;
        return STORE_PROCESS_FAIL;
    }

    radio_lock();

    if (!write_file(id_to_file(last_id, filename), pb_buffer, pb_ostream.bytes_written)) {
        DEBUG("Failed to save reading %d\n", last_id);
        last_id--;
        radio_release();
        return STORE_PROCESS_FAIL;
    }

    radio_release();

    return last_id;
}

bool get_sample(int16_t id, Sample *sample) {

    if (get_raw_sample(id, pb_buffer)) {
        return false;
    }

    pb_istream = pb_istream_from_buffer(pb_buffer, sizeof(pb_buffer));
    if (!pb_decode_delimited(&pb_istream, Sample_fields, sample)) {
        return false;
    }

    return true;
}

bool get_raw_sample(int16_t id, uint8_t buffer[Sample_size]) {

    DEBUG("Attempting to get sample %d\n", id);

    radio_lock();

    fd = cfs_open(id_to_file(id, filename), CFS_READ);

    if (fd < 0) {
        DEBUG("Failed to open file %d\n", last_id);

        radio_release();

        return false;
    }

    bytes = cfs_read(fd, buffer, Sample_size);

    DEBUG("%d bytes read\n", bytes);

    if (bytes < 0) {
        cfs_close(fd);
        radio_release();
        return false;
    }

    cfs_close(fd);
    radio_release();

    return true;
}

bool delete_sample(int16_t sample) {

    if (sample < 1) {
        DEBUG("Attempting to delete invalid sample %d\n", sample);
        return false;
    }

    DEBUG("Attempting to delete sample %d\n", sample);

    id_to_file(sample, filename);

    radio_lock();

    if (cfs_remove(filename) == -1) {
        DEBUG("Error deleting sample %d\n", sample);
        radio_release();
        return false;
    }

    // If this file used to be last_id, find the previous existing file (if any)
    if (sample == last_id) {
        DEBUG("Sample %d is last known sample. Searching for previous sample...\n", sample);

        last_id--;

        id_to_file(last_id, filename);

        // Keep going until we reach a file that exists
        while (last_id != 0 && (fd = cfs_open(filename, CFS_READ)) < 0) {
            DEBUG("Sample %d does not exist, continuing..\n", last_id);
            last_id--;
            id_to_file(last_id, filename);
        }

        cfs_close(fd);
    }

    DEBUG("Sample %d deleted. Last_id is now %d\n", sample, last_id);

    radio_release();

    return true;
}

bool save_config(SensorConfig *config) {

    DEBUG("Attempting to save config\n");

    pb_ostream = pb_ostream_from_buffer(pb_buffer, sizeof(pb_buffer));

    if (!pb_encode_delimited(&pb_ostream, SensorConfig_fields, config)) {
        return false;
    }

    radio_lock();

    if (!write_file(CONFIG_FILENAME, pb_buffer, pb_ostream.bytes_written)) {
        radio_release();
        return false;
    }

    radio_release();

    return true;
}

bool get_config(SensorConfig *config) {

    if (!get_raw_config(pb_buffer)) {
        return false;
    }

    pb_istream = pb_istream_from_buffer(pb_buffer, sizeof(pb_buffer));

    return pb_decode_delimited(&pb_istream, SensorConfig_fields, config);
}

bool get_raw_config(uint8_t buffer[SensorConfig_size]) {

    DEBUG("Attempting to get config\n");

    radio_lock();

    fd = cfs_open(CONFIG_FILENAME, CFS_READ);

    if (fd < 0) {
        DEBUG("Failed to read config file %s\n", CONFIG_FILENAME);

        radio_release();

        return false;
    }

    bytes = cfs_read(fd, buffer, SensorConfig_size);

    DEBUG("%d bytes read\n", bytes);

    if (bytes < 0) {
        cfs_close(fd);
        radio_release();
        return false;
    }

    cfs_close(fd);
    radio_release();

    return true;
}

bool write_file(char* filename, uint8_t *buffer, int length) {

    if (cfs_coffee_reserve(filename, length) < 0) {
        DEBUG("Failed to reserve space for file %s\n", filename);
    }

    fd = cfs_open(filename, CFS_WRITE);

    if (fd < 0) {
        DEBUG("Failed to open file %s for writing\n", filename);
        return false;
    }

    bytes = cfs_write(fd, buffer, length);

    if (bytes != length) {
        DEBUG("Failed to write file to %s, wrote %d bytes\n", filename, bytes);
        return false;
    }

    DEBUG("%d bytes written\n", bytes);
    cfs_close(fd);
    return true;
}

void store_init(void) {
    radio_lock();
    last_id = find_latest_sample();
    radio_release();
}

void radio_lock(void) {
#ifdef SPI_LOCKING
    NETSTACK_MAC.off(0);
    cc1120_arch_interrupt_disable();
    CC1120_LOCK_SPI();
    DEBUG("Radio Locked\n");
#endif
}

void radio_release(void) {
#ifdef SPI_LOCKING
    CC1120_RELEASE_SPI();
    cc1120_arch_interrupt_enable();
    NETSTACK_MAC.on();
    DEBUG("Radio Unlocked\n");
#endif
}

int16_t find_latest_sample(void) {
    struct cfs_dirent dirent;
    struct cfs_dir dir;
    int16_t file_num;
    int16_t max_num;

    max_num = 0;
    DEBUG("Refreshing filename cache\n");

    if (cfs_opendir(&dir, "/") == 0) {
        DEBUG("\tOpened folder\n");

        while (cfs_readdir(&dir, &dirent) != -1) {
            if (strncmp(dirent.name, FILENAME_PREFIX, 1) == 0) {
                file_num = atoi(dirent.name + 1);
                DEBUG("Filename %d found\n", file_num);
                DEBUG("\tMax: %d Filenum: %d\n", max_num, file_num);
                if(file_num > max_num) {
                    max_num = file_num;
                }
            }
        }
        return max_num;
    }
    return 0;
}

char* id_to_file(int16_t id, char* filename) {
    sprintf(filename, FILENAME_PREFIX "%d", id);
    return filename;
}

int16_t store_save_sample(Sample *sample) {
    process_post_synch(&store_process, STORE_EVENT_SAVE_SAMPLE, sample);
    // Return -1 on failure
    return *((int16_t *) sample) == STORE_PROCESS_FAIL ? -1 : *((int16_t *) sample);
}

bool store_get_sample(int16_t id, Sample *sample) {
    *((int16_t *) sample) = id;
    process_post_synch(&store_process, STORE_EVENT_GET_SAMPLE, sample);
    return *((int16_t *) sample) != STORE_PROCESS_FAIL;
}

bool store_get_raw_sample(int16_t id, uint8_t buffer[Sample_size]) {
    *((int16_t *) buffer) = id;
    process_post_synch(&store_process, STORE_EVENT_GET_RAW_SAMPLE, buffer);
    return *((int16_t *) buffer) != STORE_PROCESS_FAIL;
}

bool store_get_latest_sample(Sample *sample) {
    process_post_synch(&store_process, STORE_EVENT_GET_LATEST_SAMPLE, sample);
    return *((int16_t *) sample) != STORE_PROCESS_FAIL;
}

bool store_get_latest_raw_sample(uint8_t buffer[Sample_size]) {
    process_post_synch(&store_process, STORE_EVENT_GET_LATEST_RAW_SAMPLE, buffer);
    return *((int16_t *) buffer) != STORE_PROCESS_FAIL;
}

bool store_delete_sample(int16_t *id) {
    process_post_synch(&store_process, STORE_EVENT_DELETE_SAMPLE, id);
    return *id != STORE_PROCESS_FAIL;
}

bool store_save_config(SensorConfig *config) {
    process_post_synch(&store_process, STORE_EVENT_SAVE_CONFIG, config);
    return *((int16_t *) config) != STORE_PROCESS_FAIL;
}

bool store_get_config(SensorConfig *config) {
    process_post_synch(&store_process, STORE_EVENT_GET_CONFIG, config);
    return *((int16_t *) config) != STORE_PROCESS_FAIL;
}

bool store_get_raw_config(uint8_t buffer[SensorConfig_size]) {
    process_post_synch(&store_process, STORE_EVENT_GET_RAW_CONFIG, buffer);
    return *((int16_t *) buffer) != STORE_PROCESS_FAIL;
}

