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

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <contiki.h>
#include "ds3231-sensor.h"
#include "i2cmaster.h"


//#define DEBUG
#ifdef DEBUG
#define dprintf(FORMAT, args...) printf_P(PSTR(FORMAT), ##args)
#else
#define dprintf(...)
#endif


/**
 * DS3231 is connected to TWI Master on Port C (Pins 0 & 1). The active low
 * Interrupt from the DS3231 is connected in Pin 2 of Port C.
 */
//#define I2C(a,b,c,d)	twi_transaction(&TWIC, DS3231_ADDR, (a), (b), (c), (d));

int I2C(uint8_t* write,uint16_t write_len,uint8_t* read,uint16_t read_len)
{
	if (write_len) {
		i2c_transmitinit(DS3231_ADDR);
                while(i2c_busy());
		i2c_transmit_n(write_len,write);
                while(i2c_busy());
	}

	if (read_len) {
		i2c_receiveinit(DS3231_ADDR);
                while(i2c_busy());
		i2c_receive_n(read_len,read);
                while(i2c_busy());
	}

	return(0);
}

/* Define the sensor object. */
const struct sensors_sensor ds3231_sensor;

/* Sensor status. */
enum {
	ON,
	OFF
};
static uint8_t state = OFF;

static int accum_days[] = {
		0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
};

/**
 * ds3231_get_epoch_seconds
 *
 * @return	The number of seconds since the Unix epoch, or 0 in case of error.
 *
 * @note	Unix epoch is taken as 1970-01-01 00:00:00 UTC.
 *
 * @note	In this application the base date stored in the RTC
 * 			is from 2000-01-01.
 *
 * @note	When accessed via the 'value' method, for the MSB and LSB, the
 * 			return value will need to be cast from int to uint_16t and shifted
 * 			accordingly.
 */
uint32_t ds3231_get_epoch_seconds(void)
{
	int rc;
	int years, months, minutes, seconds;
	uint32_t days, hours, epoch;
	ds_3231_time_t time;

	/* Read the time from the DS3231. */
	time.tm.address = 0;
	rc = I2C(time.data, 1, &time.data[1], 7);
	if (rc != 0) {
		return 0;
	}

	/* Convert to epoch seconds. */
	years = time.tm.year + time.tm.dyear * 10 + 30;
	months = time.tm.mon + time.tm.dmon * 10 - 1;
	days = time.tm.date + time.tm.ddate * 10 - 1;
	days += accum_days[months] + years * 365 + ((years + 2) / 4);
	days += ((years + 3) % 4 == 0) && (months > 1) ? 1 : 0;
	hours = time.tm.hour + time.tm.dhour * 10;
	minutes = time.tm.min + time.tm.dmin * 10;
	seconds = time.tm.sec + time.tm.dsec * 10;
	epoch = days * 86400 + hours * 3600 + minutes * 60 + seconds;

	dprintf("years %d, months %d, days %lu, hours %lu, minutes %d, seconds %d\n",
			years, months, days, hours, minutes, seconds);
	dprintf("epoch is %lu\n", epoch);

	return epoch;
}

/**
 * ds3231_set_time
 *
 * @return	0 for success, -ve for error.
 *
 * @note	The base date of tm struct is 1900-01-01 and the base date stored
 * 			in the RTC for this application is 2000-01-01. An adjustment of
 * 			100 is made before setting the time.
 */
int ds3231_set_time(tm *t)
{
	int rc;
	ds_3231_time_t time;

	memset(time.data, 0, 8);

	/* Convert the tm struct to the register representation of the DS3231. */
	time.tm.dyear = (t->tm_year - 100) / 10;
	time.tm.year = (t->tm_year - 100) % 10;
	time.tm.dmon = (t->tm_mon + 1) / 10;
	time.tm.mon = (t->tm_mon + 1) % 10;
	time.tm.ddate = t->tm_mday / 10;
	time.tm.date = t->tm_mday % 10;
	time.tm.day = 1; /* Not used, set to 1. */
	time.tm.dhour = t->tm_hour / 10;
	time.tm.hour = t->tm_hour % 10;
	time.tm.dmin = t->tm_min / 10;
	time.tm.min = t->tm_min % 10;
	time.tm.dsec = t->tm_sec / 10;
	time.tm.sec = t->tm_sec % 10;

	/* Send the new time to the DS3231. */
	rc = I2C(time.data, 8, NULL, 0);
	if (rc != 0) {
		return rc;
	}

	return 0;
}

/**
 * ds3231_set_alarm
 *
 * Sets the first alarm of the DS3231. Alarm Mask Bits A1M1-4 are all set
 * to 0 which means the alarm will fire on the complete alarm setting (date,
 * hour, minute & second).
 *
 * When the alarm is triggered the INT pin of the DS3231 will be pulled low.
 * This pin is connected to Port C Pin 2 of the AVR XMega.
 *
 * @return	0 for success, -ve for error.
 */
int ds3231_set_alarm(tm *t)
{
	int rc;
	ds_3231_alarm_t alarm;
	uint8_t regs[3];

	memset(alarm.data, 0, 5);

	/* Set the Alarm 1 registers. */
	alarm.tm.address = 7; /* DS3231 register 7, start of Alarm 1. */
	alarm.tm.ddate = t->tm_mday / 10;
	alarm.tm.date = t->tm_mday % 10;
	alarm.tm.dhour = t->tm_hour / 10;
	alarm.tm.hour = t->tm_hour % 10;
	alarm.tm.dmin = t->tm_min / 10;
	alarm.tm.min = t->tm_min % 10;
	alarm.tm.dsec = t->tm_sec / 10;
	alarm.tm.sec = t->tm_sec % 10;

	/* Send the Alarm to the DS3231. */
	rc = I2C(alarm.data, 5, NULL, 0);
	if (rc != 0) {
		return rc;
	}

	/* Read in Control and Status registers. */
	regs[0] = 0x0e;
	rc = I2C(&regs[0], 1, &regs[1], 2);
	if (rc != 0) {
		return rc;
	}

	/* Enable Alarm 1, then write the Control and Status registers. */
	regs[1] |= DS3231_CONTROL_A1IE_SET_MASK;
	regs[2] &= DS3231_STATUS_A1F_CLEAR_MASK;
	rc = I2C(regs, 3, NULL, 0);
	if (rc != 0) {
		return rc;
	}

	return 0;
}

/**
 * ds3231_clear_alarm
 *
 * Clears and disables Alarm 1.
 *
 * @return	0 for success, -ve for error.
 */
int ds3231_clear_alarm(void)
{
	int rc;
	uint8_t regs[3];

	/* Read in Control and Status registers. */
	regs[0] = 0x0e;
	rc = I2C(&regs[0], 1, &regs[1], 2);
	if (rc != 0) {
		return rc;
	}

	/* Write the Control and Status registers. */
	regs[1] &= DS3231_CONTROL_A1IE_CLEAR_MASK;
	regs[2] &= DS3231_STATUS_A1F_CLEAR_MASK;
	rc = I2C(regs, 3, NULL, 0);
	if (rc != 0) {
		return rc;
	}

	return 0;
}

/**
 * ds3231_temperature
 *
 * Register 11h: bit 7 is sign, bit 6-0 are temperature.
 * Register 12h: bit 7 & 6 are decimal point (0.25 * xxb).
 *
 * @return	Temperature * 100 (centi %) however accuracy is limited to
 * 		0.25 degree increments.
 */
int ds3231_temperature(void)
{
	int rc;
	uint8_t addr_set[6] = { 0x11 };
	uint8_t temp_regs[21];
	int temperature;

	rc = I2C(addr_set, 1, temp_regs, 2);
	if (rc != 0) {
		return 0;
	}

	temperature = ((temp_regs[0]) & 0x7f) * 100; /* Temperature in cC. */
	temperature += ((temp_regs[1] & 0xc0) >> 6) * 25; /* .25 .. .75 */
	if (temp_regs[0] & 0x80)
		temperature *= -1;

	dprintf("Temperature is %d\n", temperature);

	return temperature;
}

/**
 * value
 */
int value(int type)
{
	switch (type) {
	case DS3231_SENSOR_TEMP:
		return ds3231_temperature();
	case DS3231_SENSOR_GET_EPOCH_SECONDS_MSB:
		return (int)(ds3231_get_epoch_seconds() >> 16);
	case DS3231_SENSOR_GET_EPOCH_SECONDS_LSB:
		return (int)(ds3231_get_epoch_seconds() & 0xffff);
	default:
		break;
	}

	return 0;
}

/**
 * configure
 */
int configure(int type, int c)
{
	int rc = 0;
	uint8_t addr_set[1] = { 0 };
	uint8_t register_map[19];

	switch (type) {
	case DS3231_CONFIG_SET_TIME:
		/* The address of a tm structure is pass via variable
		 * 'c' which works because sizeof(tm*) and sizeof(int)
		 * are the same on AVR XMEGA. Its generally not a good
		 * thing to do ... things like that. */
		return ds3231_set_time((tm*) c);
	case DS3231_CONFIG_SET_ALARM:
		return ds3231_set_alarm((tm*) c);
	case DS3231_CONFIG_CLEAR_ALARM:
		return ds3231_clear_alarm();
	case SENSORS_ACTIVE:
		if (c) {
			state = OFF;
			dprintf("DS3231 Sensor Configure ...\n");
			rc = ds3231_clear_alarm();
			if (rc == 0)
				state = ON;
			break;
		}
	default:
		state = OFF;
	}

	return 0;
}

/**
 * status
 */
int status(int type)
{
	switch (type) {
	case SENSORS_ACTIVE:
	case SENSORS_READY:
		return (state == ON);
	default:
		return 0;
	}
}


/* Initialise the sensor object and make it available to Contiki OS. */
SENSORS_SENSOR(ds3231_sensor, "ds3231", value, configure, status);
