#ifndef SAMPLING_SENSORS_H
#define SAMPLING_SENSORS_H
	uint16_t get_sensor_rain(void);
	uint16_t get_sensor_ADC1(void);
	uint16_t get_sensor_ADC2(void);
	float get_sensor_temp(void);
	float get_sensor_batt(void);
	int16_t get_sensor_acc_x(void);
	int16_t get_sensor_acc_y(void);
	int16_t get_sensor_acc_z(void);
	uint32_t get_time(void);

	uint8_t set_time(uint16_t y, uint8_t mo, uint8_t d, uint8_t h, uint8_t mi, uint8_t s);
#endif
