// Sensors
#include "dev/uart1_i2c_master.h"

#include "dev/ds3231-sensor.h" 	// Clock
#include "dev/adc1-sensor.h" 	// ADC 1
#include "dev/adc2-sensor.h" 	// ADC 2
#include "dev/temperature-sensor.h" // Temp
#include "dev/batv-sensor.h" // Batt
#include "adxl345.h" 		// Accel
#include "dev/event-sensor.h"	//event sensor (rain)
#include "sampling-sensors.h"

#define ADC_ACTIVATE_DELAY 10 //delay in ticks of the rtimer  PLATFORM DEPENDANT!

//#define NO_RTC

uint16_t 
get_sensor_rain(void)
{
    return event_sensor.value(1);
}

uint16_t 
get_sensor_ADC1(void)
{
    uint16_t adc1_ret;
    rtimer_clock_t t0;
    SENSORS_ACTIVATE(adc1_sensor);
    t0 = RTIMER_NOW();
    while(RTIMER_CLOCK_LT(RTIMER_NOW(), (t0 + (uint32_t) ADC_ACTIVATE_DELAY)));
    adc1_ret =  adc1_sensor.value(0);
    SENSORS_DEACTIVATE(adc1_sensor);
    return adc1_ret;
}

uint16_t 
get_sensor_ADC2(void)
{
    uint16_t ret;
    rtimer_clock_t t0;
    SENSORS_ACTIVATE(adc2_sensor);
    t0 = RTIMER_NOW();
    while(RTIMER_CLOCK_LT(RTIMER_NOW(), (t0 + (uint32_t) ADC_ACTIVATE_DELAY)));
    ret =  adc2_sensor.value(0);
    SENSORS_DEACTIVATE(adc2_sensor);
    return ret;
}

float 
get_sensor_temp(void)
{
    return (float)(((temperature_sensor.value(0)*2.500)/4096)-0.986)*282;
}

float 
get_sensor_batt(void)
{
    float bat_ret;
    rtimer_clock_t t0;
    SENSORS_ACTIVATE(batv_sensor);
    t0 = RTIMER_NOW();
    while(RTIMER_CLOCK_LT(RTIMER_NOW(), (t0 + (uint32_t) ADC_ACTIVATE_DELAY)));
    bat_ret =  (float)(batv_sensor.value(0)) / 273.067;
    SENSORS_DEACTIVATE(batv_sensor);
    return bat_ret;
}

int16_t 
get_sensor_acc_x(void)
{
    return accm_read_axis(X_AXIS);
}

int16_t 
get_sensor_acc_y(void)
{
    return accm_read_axis(Y_AXIS);
}

int16_t 
get_sensor_acc_z(void)
{
   return accm_read_axis(Z_AXIS);
}

uint32_t 
get_time(void)
{
#ifdef NO_RTC
    return (uint32_t)12345;
#else
    return ds3231_get_epoch_seconds();
#endif
}

bool
set_time(uint32_t seconds)
{
    return ds3231_set_epoch_seconds(seconds);
}
