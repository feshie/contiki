/*
 * Copyright (c) 2012, Timothy Rule <trule.github@nym.hush.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/**
 * @file
 * 		Sensors for DS3231 (RTC with Temperature Sensor).
 * @author
 * 		Timothy Rule <trule.github@nym.hush.com>
 * @note
 * 		See http://www.maxim-ic.com/datasheet/index.mvp/id/4627
 */

#ifndef __DS3231_SENSOR__
#define __DS3231_SENSOR__

#include <lib/sensors.h>

/**
 * DS3231 Sensor and Config Command List.
 */
#define DS3231_SENSOR_TEMP						0
#define DS3231_SENSOR_GET_EPOCH_SECONDS_MSB		1
#define DS3231_SENSOR_GET_EPOCH_SECONDS_LSB		2
#define DS3231_CONFIG_SET_TIME					55
#define DS3231_CONFIG_SET_ALARM					56
#define DS3231_CONFIG_CLEAR_ALARM				57

#define DS3231_MASTER					0x00
#define DS3231_ADDR			0x68 //0xd0 with shift
#define DS3231_CONTROL_A1IE_SET_MASK	0x05
#define DS3231_CONTROL_A1IE_CLEAR_MASK	0xfe
#define DS3231_STATUS_A1F_CLEAR_MASK	0xfe

/**
 * tm
 *
 * An abbreviated version of tm struct from time.h. See Open Group for full
 * documentation of this structure.
 */
typedef struct {
	int tm_sec; 	// seconds [0,61]
	int tm_min; 	// minutes [0,59]
	int tm_hour; 	// hour [0,23]
	int tm_mday; 	// day of month [1,31]
	int tm_mon; 	// month of year [0,11]
	int tm_year; 	// years since 1900
} tm;

typedef union {
	struct {
		/* Start Address on RTC for time registers. */
		uint8_t address;
		/* Byte 0 */
		uint8_t sec : 4;
		uint8_t dsec : 3;
		uint8_t : 1;
		/* Byte 1 */
		uint8_t min : 4;
		uint8_t dmin : 3;
		uint8_t : 1;
		/* Byte 2 */
		uint8_t hour : 4;
		uint8_t dhour : 2;
		uint8_t : 2; /* Runs 24 hour time (default). */
		/* Byte 3 */
		uint8_t day : 3;
		uint8_t : 5;
		/* Byte 4 */
		uint8_t date : 4;
		uint8_t ddate : 2;
		uint8_t : 2;
		/* Byte 5 */
		uint8_t mon : 4;
		uint8_t dmon : 1;
		uint8_t : 2;
		uint8_t cent : 1;
		/* Byte 6 */
		uint8_t year : 4;
		uint8_t dyear : 4;
	} tm;
	uint8_t data[8];
} ds_3231_time_t;

typedef union {
	struct {
		/* Start Address on RTC for alarm registers. */
		uint8_t address;
		/* Byte 0 */
		uint8_t sec : 4;
		uint8_t dsec : 3;
		uint8_t a1m1 : 1;
		/* Byte 1 */
		uint8_t min : 4;
		uint8_t dmin : 3;
		uint8_t a1m2: 1;
		/* Byte 2 */
		uint8_t hour : 4;
		uint8_t dhour : 2;
		uint8_t : 1; /* Runs 24 hour time (default). */
		uint8_t a1m3 : 1;
		/* Byte 3 */
		uint8_t date : 4;
		uint8_t ddate : 2;
		uint8_t : 1; /* Date based alarm (default) */
		uint8_t a1m4 : 1;
	} tm;
	uint8_t data[5];
} ds_3231_alarm_t;

/**
 * Export the DS3231 sensor object.
 *
 * Can be called as follows:
 *
 * 		#include <dev/ds3231-sensor.h>
 *
 * 		SENSORS_ACTIVATE(ds3231_sensor);
 *		ds3231_sensor.configure(DS3231_CONFIG_SET_TIME, (int)&t);
 *		ds3231_sensor.configure(DS3231_CONFIG_SET_ALARM, (int)&t);
 *		ds3231_sensor.value(DS3231_SENSOR_TEMP);
 *		ds3231_sensor.value(DS3231_SENSOR_GET_EPOCH_SECONDS_LSB);
 *		ds3231_sensor.value(DS3231_SENSOR_GET_EPOCH_SECONDS_MSB);
 *		ds3231_sensor.configure(DS3231_CONFIG_CLEAR_ALARM, 0);
 */
extern const struct sensors_sensor ds3231_sensor;

uint32_t ds3231_get_epoch_seconds(void);
int ds3231_set_time(tm *t);
int ds3231_set_alarm(tm *t);
int ds3231_clear_alarm(void);
int ds3231_temperature(void);
int value(int type);
int configure(int type, int c);
int status(int type);

#endif
