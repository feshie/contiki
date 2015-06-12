ifndef TARGET
TARGET=z1-feshie
endif

CONTIKI_PROJECT = test-adxl345 tmp102-test batv-test adc1-test adc2-test \
      adcs-test ds323-test event-test rs485-test
ROJECT_SOURCEFILES = i2cmaster.c tmp102.c adxl345.c sky-sensors.c batv-sensor.c
APPS=serial-shell
 
all: $(CONTIKI_PROJECT)
 
CONTIKI = ../..
include $(CONTIKI)/Makefile.include
