CONTIKI = ../..
ifndef TARGET
TARGET=z1-feshie
endif

all: cfs-ls cfs-cat

%.tgz: %.ihex
	mkdir $(basename $<) ; \
	mv $< $(basename $<) ; \
	echo $(basename $<)/$(basename $<).ihex 600 > $(basename $<)/runfile ; \
	tar czf $@ $(basename $<)

include $(CONTIKI)/Makefile.include
