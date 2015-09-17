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

/**
 * Single char prefix to use for filenames
 */
#define FILENAME_PREFIX "r"

/**
 * Filename of the config file
 */
#define CONFIG_FILENAME "config"

/**
 * Maximum length of a file name
 */
#define FILENAME_LENGTH 8

/**
 * Magic value we append to all files we write.
 * This ensures trailing NULL bytes are not removed when the
 * file is read from flash again (this only happens if
 * the file isn't cached - ie on a fresh boot, or if it hasn't
 * been openned in a while).
 */
static const uint8_t END_CANARY = 0xAF;

/**
 * Identifier of the last sample.
 */
static uint16_t last_id;

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
static uint16_t find_latest_sample(void);

/**
 * Read a given file.
 * @return The number of bytes read succesfully from filename, or false if the file could not be read.
 * NOTE: This will strip the trailing canary byte appended by `write_file`.
 */
static uint8_t read_file(char *filename, uint8_t *buffer, uint8_t length);

/**
 * Write to a given file.
 * @return true if length bytes have been successfully written to filename, false otherwise.
 * NOTE: This will append a trailing canary byte that will be stripped by `read_file`.
 */
static bool write_file(char *filename, uint8_t *buffer, uint8_t length);

/**
 * Convert a sample id to a filename.
 * @return The pointer to filename(usefull for avoiding temp vars).
 */
static char* id_to_file(uint16_t id, char* filename);

uint16_t store_save_sample(Sample *sample) {
    pb_ostream_t pb_ostream;
    uint8_t pb_buffer[Sample_size];
    char filename[FILENAME_LENGTH];

    last_id++;

    sample->id = last_id;

    DEBUG("Attempting to save reading with id %d\n", last_id);

    pb_ostream = pb_ostream_from_buffer(pb_buffer, sizeof(pb_buffer));
    if (!pb_encode_delimited(&pb_ostream, Sample_fields, sample)) {
        last_id--;
        return false;
    }

    radio_lock();

    if (!write_file(id_to_file(last_id, filename), pb_buffer, pb_ostream.bytes_written)) {
        DEBUG("Failed to save reading %d\n", last_id);
        last_id--;
        radio_release();
        return false;
    }

    radio_release();

    return last_id;
}

bool store_get_latest_sample(Sample *sample) {
    return store_get_sample(last_id, sample);
}

uint8_t store_get_latest_raw_sample(uint8_t buffer[Sample_size]) {
    return store_get_raw_sample(last_id, buffer);
}

bool store_get_sample(uint16_t id, Sample *sample) {
    pb_istream_t pb_istream;
    uint8_t pb_buffer[Sample_size];

    if (store_get_raw_sample(id, pb_buffer)) {
        return false;
    }

    pb_istream = pb_istream_from_buffer(pb_buffer, sizeof(pb_buffer));
    if (!pb_decode_delimited(&pb_istream, Sample_fields, sample)) {
        return false;
    }

    return true;
}

uint8_t store_get_raw_sample(uint16_t id, uint8_t buffer[Sample_size]) {
    char filename[FILENAME_LENGTH];
    uint8_t bytes;

    DEBUG("Attempting to get sample %d\n", id);

    radio_lock();

    bytes = read_file(id_to_file(id, filename), buffer, Sample_size);

    radio_release();

    return bytes;
}

uint16_t store_get_latest_sample_id(void) {
    return last_id;
}

bool store_delete_sample(uint16_t sample) {
    int fd = 0;
    char filename[FILENAME_LENGTH];

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

bool store_save_config(SensorConfig *config) {
    pb_ostream_t pb_ostream;
    uint8_t pb_buffer[SensorConfig_size];

    DEBUG("Attempting to save config\n");

    pb_ostream = pb_ostream_from_buffer(pb_buffer, sizeof(pb_buffer));

    if (!pb_encode_delimited(&pb_ostream, SensorConfig_fields, config)) {
        return false;
    }

    radio_lock();

    // Delete the old config first, avoids the need for micro-logs
    if (cfs_remove(CONFIG_FILENAME) == -1) {
        DEBUG("Error deleting old config\n");
    }

    if (!write_file(CONFIG_FILENAME, pb_buffer, pb_ostream.bytes_written)) {
        radio_release();
        return false;
    }

    radio_release();

    return true;
}

bool store_get_config(SensorConfig *config) {
    pb_istream_t pb_istream;
    uint8_t pb_buffer[SensorConfig_size];

    if (!store_get_raw_config(pb_buffer)) {
        return false;
    }

    pb_istream = pb_istream_from_buffer(pb_buffer, sizeof(pb_buffer));

    return pb_decode_delimited(&pb_istream, SensorConfig_fields, config);
}

uint8_t store_get_raw_config(uint8_t buffer[SensorConfig_size]) {
    uint8_t bytes;

    DEBUG("Attempting to get config\n");

    radio_lock();

    bytes = read_file(CONFIG_FILENAME, buffer, SensorConfig_size);

    radio_release();

    return bytes;
}

uint8_t read_file(char *filename, uint8_t *buffer, uint8_t length) {
    int fd = cfs_open(filename, CFS_READ);

    if (fd < 0) {
        DEBUG("Failed to open file %s\n", filename);
        return false;
    }

    uint8_t bytes = cfs_read(fd, buffer, length);

    cfs_close(fd);

    if (bytes <= 0) {
        DEBUG("Failed to read file %s\n", filename);
        return false;
    }

    // Ignore the end canary value if there is one (backwards compability for files that don't have a canary)
    if (buffer[bytes-1] == END_CANARY) {
        bytes--;
    }

    DEBUG("%d bytes read\n", bytes);

    return bytes;
}

bool write_file(char *filename, uint8_t *buffer, uint8_t length) {
    int fd;
    int bytes;

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

    // Write the trailing magic value
    if (cfs_write(fd, &END_CANARY, 1) == -1) {
        DEBUG("Failed to write trailing canary to file %s\n", filename);
        return false;
    }

    DEBUG("%d bytes written\n", bytes);
    cfs_close(fd);
    return true;
}

void store_init(void) {
    DEBUG("Initializing...\n");
    radio_lock();
    last_id = find_latest_sample();
    radio_release();
    printf("Store initialized. %d previous files found.\n", last_id);
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

uint16_t find_latest_sample(void) {
    struct cfs_dirent dirent;
    struct cfs_dir dir;
    uint16_t file_num;
    uint16_t max_num;

    max_num = 0;
    DEBUG("Refreshing filename cache\n");

    if (cfs_opendir(&dir, "/") == 0) {
        DEBUG("\tOpened folder\n");

        while (cfs_readdir(&dir, &dirent) != -1) {
            if (strncmp(dirent.name, FILENAME_PREFIX, 1) == 0) {
                file_num = atoi(dirent.name + 1);
                //DEBUG("Filename %d found\n", file_num);
                //DEBUG("\tMax: %d Filenum: %d\n", max_num, file_num);
                if(file_num > max_num) {
                    max_num = file_num;
                }
            }
        }
        return max_num;
    }
    return 0;
}

char* id_to_file(uint16_t id, char* filename) {
    sprintf(filename, FILENAME_PREFIX "%d", id);
    return filename;
}
