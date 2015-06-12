CONTIKI = ../..
ifndef TARGET
TARGET=z1-feshie
endif

all: node-setup

%.tgz: %.ihex
	mkdir $(basename $<) ; \
	mv $< $(basename $<) ; \
	echo $(basename $<)/$(basename $<).ihex 600 > $(basename $<)/runfile ; \
	tar czf $@ $(basename $<)

%.class: %.java
	javac $(basename $<).java

install: z1.feshie.upload

viewrssi: ViewRSSI.class
	make login | java ViewRSSI

include $(CONTIKI)/Makefile.include
