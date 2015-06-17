#include "store.h"
#include "stdio.h"
#include "stdlib.h"
#include "contiki.h"
#include "pt-sem.h"
#include "cfs/cfs.h"
#include "pb_decode.h"
#include "pb_encode.h"

#ifdef SPI_LOCKING
    #include "dev/cc1120.h"
    #include "dev/cc1120-arch.h"
#endif

#define STORE_DEBUG
#ifdef STORE_DEBUG
    #define FPRINT(...) printf(__VA_ARGS__)
#else
    #define FPRINT(...)
#endif

PROCESS(store_process, "Store Process");

/**
 * Single char prefix to use for filenames
 */
#define FILENAME_PREFIX "r"

/**
 * Filename of the config file
 */
#define CONFIG_FILENAME "sampleconfig"

/*
 * Size is set to Sample_size, as Sample is assumed to be bigger than config.
 */
#define PB_BUF_SIZE (Sample_size)

/**
 * Maximum length of a file name
 */
#define FILENAME_LENGTH 8

/**
 * Magic canary to indicate that processing the given event has failed.
 */
#define STORE_PROCESS_FAIL      (-456)

/**
 * Data should point to sample to save
 * Data will point to a int16_t >= 0 on success, STORE_PROCESS_FAIL otherwise.
 */
#define STORE_EVENT_SAVE_SAMPLE         1

/**
 * Data should point to a int16_t (but needs to point to a chunk of memory Sample sized)
 * Data will point to the sample with that id on success, STORE_PROCESS_FAIL otherwise.
 */
#define STORE_EVENT_GET_SAMPLE          2

/**
 * Data should point to a chunk of memory Sample sized.
 * Data will point to the lastest sample on success, STORE_PROCESS_FAIL otherwise.
 */
#define STORE_EVENT_GET_LATEST_SAMPLE   3

/**
 * Data should point to a int16_t
 * Data will point to STORE_PROCESS_FAIL on failure.
 */
#define STORE_EVENT_DELETE_SAMPLE       4

/**
 * Data should point the SampleConfig to save
 * Data will point to STORE_PROCESS_FAIL on failure.
 */
#define STORE_EVENT_SAVE_CONFIG         5

/**
 * Data should point to a chunk of memory SampleConfig sized
 * Data will point to the SampleConfig on success, STORE_PROCESS_FAIL otherwise.
 */
#define STORE_EVENT_GET_CONFIG          6

/**
 * Identifier of the last sample.
 */
static int16_t last_id;

/**
 * Protocol Buffer output_stream
 */
static pb_ostream_t pb_ostream;

/**
 * Protocol Buffer input_stream
 */
static pb_istream_t pb_istream;

/**
 * Protocol Buffer buffer to use for enconding / decoding.
 */
static uint8_t pb_buffer[PB_BUF_SIZE];

/**
 * File descriptor to use.
 */
static int fd;

/**
 * Buffer for filename to use
 */
static char filename[FILENAME_LENGTH];

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

static int16_t save_sample(Sample *sample);

//static bool get_sample(int16_t id, Sample *sample);
//static bool delete_sample(int16_t sample);
//static bool save_config(SampleConfig *config);
//static bool get_config(SampleConfig *config);

/**
 * Convert a sample id to a filename.
 */
static char* id_to_file(int16_t id, char* filename);


PROCESS_THREAD(store_process, ev, data) {
    // static vars and what not

    PROCESS_BEGIN();

    FPRINT("[STORE] Initializing...\n");

    store_init();

    FPRINT("[STORE] Started\n");

    while (true) {
        PROCESS_WAIT_EVENT();

        switch(ev) {

            case STORE_EVENT_SAVE_SAMPLE:
                *((int16_t *) data) = save_sample((Sample *) data);
                break;

            case STORE_EVENT_GET_SAMPLE:
                //*((int16_t *) data) = get_sample(*((int16_t *) data), (Sample *) data);
                break;

            case STORE_EVENT_GET_LATEST_SAMPLE:
                /*
                static bool ret;

                // Loop around to avoid files "marked" as deleted
                while (!(ret = store_get_reading(pt, last_id, sample))) {}

                return ret;*/

                break;

            case STORE_EVENT_DELETE_SAMPLE:
                /*//store_lock(pt);

                // If id == last_id, properly delete the file.
                if (id == last_id) {
                    cfs_remove(id_to_file(id, filename));
                    last_id--;

                // Otherwise just set a magic value to "mark" the file as deleted
                } else {
                    // TODO
                }

                //store_release(pt);
                */
                break;

            case STORE_EVENT_SAVE_CONFIG:

                break;

            case STORE_EVENT_GET_CONFIG:

                break;

            default:
                FPRINT("[STORE] Unknown Event %d!\n", ev);
                break;
        }
    }

    PROCESS_END();
}

int16_t save_sample(Sample *sample) {
    static uint8_t bytes_written;

    last_id++;

    sample->id = last_id;

    FPRINT("[STORE] Attempting to save reading with id %d\n", last_id);

    pb_ostream = pb_ostream_from_buffer(pb_buffer, sizeof(pb_buffer));
    pb_encode_delimited(&pb_ostream, Sample_fields, sample);

    radio_lock();

    fd = cfs_open(id_to_file(last_id, filename), CFS_WRITE);

    if (fd < 0) {
        FPRINT("[STORE] Failed to create file %d\n", last_id);

        last_id--;

        radio_release();

        return STORE_PROCESS_FAIL;
    }

    bytes_written = cfs_write(fd, pb_buffer, pb_ostream.bytes_written);

    FPRINT("[STORE] %d bytes written\n", bytes_written);

    cfs_close(fd);
    radio_release();

    return last_id;
}
/*
int16_t get_sample(int16_t id, Sample *sample) {
    radio_lock();

    fd = cfs_open(id_to_file(id, filename), CFS_READ);

    if (fd < 0) {
        FPRINT("[STORE] Failed to open file %d\n", last_id);

        

        return false;
    }

    cfs_read(fd, pb_buffer, sizeof(pb_buffer));

    cfs_close(fd);
    //store_release(pt);

                return true;

}*/

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
    FPRINT("[STORE] Radio Locked\n");
#endif
}

void radio_release(void) {
#ifdef SPI_LOCKING
    CC1120_RELEASE_SPI();
    cc1120_arch_interrupt_enable();
    NETSTACK_MAC.on();
    FPRINT("[STORE] Radio Unlocked\n");
#endif
}

int16_t find_latest_sample(void) {
    static struct cfs_dirent dirent;
    static struct cfs_dir dir;
    static int16_t file_num;
    static int16_t max_num;

    max_num = 0;
    FPRINT("Refreshing filename cache\n");

    if (cfs_opendir(&dir, "/") == 0) {
        FPRINT("\tOpened folder\n");

        while (cfs_readdir(&dir, &dirent) != -1) {
            if (strncmp(dirent.name, FILENAME_PREFIX, 1) == 0) {
                file_num = atoi(dirent.name + 1);
                FPRINT("Filename %d found\n", file_num);
                FPRINT("\tMax: %d Filenum: %d\n", max_num, file_num);
                if(file_num > max_num) {
                    max_num = file_num;
                }
            }
        }

        if (max_num == 0) {
            printf("\tNo previous files found\n");
            return 0;
        } else {
            printf("\tPrevious files found.  Highest number = %d\n", max_num);
            return max_num;
        }
    }

    return 0;
}

char* id_to_file(int16_t id, char* filename) {
    sprintf(filename, FILENAME_PREFIX "%d", id);
    return filename;
}

int16_t store_save_sample(Sample *sample) {
    process_post_synch(&store_process, STORE_EVENT_SAVE_SAMPLE, sample);
    return *((int16_t *) sample);
}

bool store_get_sample(int16_t id, Sample *sample) {
    *((int16_t *) sample) = id;
    process_post_synch(&store_process, STORE_EVENT_GET_SAMPLE, sample);
    return *((int16_t *) sample) != STORE_PROCESS_FAIL;
}

bool store_get_latest_sample(Sample *sample) {
    process_post_synch(&store_process, STORE_EVENT_GET_LATEST_SAMPLE, sample);
    return *((int16_t *) sample) != STORE_PROCESS_FAIL;
}

bool store_delete_sample(int16_t id) {
    // TODO this shouldn't modify the ID on failure!
    process_post_synch(&store_process, STORE_EVENT_DELETE_SAMPLE, &id);
    return id != STORE_PROCESS_FAIL;
}

bool store_save_config(SensorConfig *config) {
    process_post_synch(&store_process, STORE_EVENT_SAVE_CONFIG, config);
    return *((int16_t *) config) != STORE_PROCESS_FAIL;
}

bool store_get_config(SensorConfig *config) {
    process_post_synch(&store_process, STORE_EVENT_GET_CONFIG, config);
    return *((int16_t *) config) != STORE_PROCESS_FAIL;
}
