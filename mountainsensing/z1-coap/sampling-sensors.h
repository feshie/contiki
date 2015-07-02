#ifndef SAMPLING_SENSORS_H
#define SAMPLING_SENSORS_H

#include <stdbool.h>

uint16_t get_sensor_rain(void);
uint16_t get_sensor_ADC1(void);
uint16_t get_sensor_ADC2(void);
float get_sensor_temp(void);
float get_sensor_batt(void);
int16_t get_sensor_acc_x(void);
int16_t get_sensor_acc_y(void);
int16_t get_sensor_acc_z(void);

/**
 * Get the current time.
 * @return  The number of seconds since the Unix epoch, or 0 in case of error.
 * @note	Unix epoch is taken as 1970-01-01 00:00:00 UTC.
 */
uint32_t get_time(void);

/**
 * Set the current time.
 * @param   seconds The number of seconds since the Unix epoch
 * @return  True on success, false otherwise
 * @note	Unix epoch is taken as 1970-01-01 00:00:00 UTC.
 */
bool set_time(uint32_t seconds);

#endif // ifndef SAMPLING_SENSORS_H
