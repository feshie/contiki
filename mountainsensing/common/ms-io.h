/**
 * Common mountain sensing interface to all sensors.
 */
#ifndef MS_IO_H
#define MS_IO_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Epoch indicating an error occured.
 */
#define ERROR_EPOCH 12345

/**
 * Initialize any hardware required for sampling,
 * and disable sense power.
 */
void ms_init(void);

/**
 * Turn on power to external sensors.
 */
void ms_sense_on(void);

/**
 * Turn off power to external sensors.
 */
void ms_sense_off(void);

/**
 * Get the current time.
 * @param seconds Pointer to write the number of seconds since the Unix epoch. On error, ERROR_EPOCH will be written to it.
 * @return True on success, False otherwise
 *
 * @note Unix epoch is taken as 1970-01-01 00:00:00 UTC.
 */
bool ms_get_time(uint32_t *seconds);

/**
 * Set the current time.
 * @param seconds The number of seconds since the Unix epoch.
 * @return True on success, False otherwise
 *
 * @note Unix epoch is taken as 1970-01-01 00:00:00 UTC.
 */
bool ms_set_time(uint32_t seconds);

/**
 * Get the battery voltage, in V.
 * @param batt Pointer to write the battery voltage to
 * @return True on success, False otherwise
 */
bool ms_get_batt(float *batt);

/**
 * Get the temperature, in degrees Celcius.
 * @param temp Pointer to write the temperature to
 * @return True on success, False otherwise
 */
bool ms_get_temp(float *temp);

/**
 * Get the humidit, in percentage.
 * @param humid Pointer to write the humidity to
 * @return True on success, False otherwise
 */
bool ms_get_humid(float *humid);

/**
 * Get the value of adc1.
 * @param adc1 Pointer to write the value of adc1 to
 * @return True on success, False otherwise
 */
bool ms_get_adc1(uint32_t *adc1);

/**
 * Get the value of adc2
 * @param adc2 Pointer to write the value of adc2 to
 * @return True on success, False otherwise
 */
bool ms_get_adc2(uint32_t *adc2);

/**
 * Get the number of rain bucket tips, and reset the count.
 * @param rain Pointer to write the rain tips to
 * @return True on success, False otherwise
 */
bool ms_get_rain(uint32_t *rain);

/**
 * Get the value of the accelerometers.
 * @param x Pointer to write the x axis acceleration to
 * @param y Pointer to write the y axis acceleration to
 * @param z Pointer to write the z axis acceleration to
 * @return True on success, False otherwise
 */
bool ms_get_acc(int32_t *x, int32_t *y, int32_t *z);

/**
 * Get the number of reboot counts.
 * Depending on the implementation, this could either be the
 * number of reboots since flash, or since ms_reset_reboot was called.
 */
bool ms_get_reboot(uint16_t *reboot);

/**
 * Reset the reboot counter.
 */
bool ms_reset_reboot(void);

#endif // #ifndef MS_IO_ARCH_H
