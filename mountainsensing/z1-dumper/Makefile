
all: z1-dumper
TARGET = z1-feshie

CONTIKI=../../

WITH_UIP6=1
UIP_CONF_IPV6=1

SMALL=1

APPS += webserver webbrowser

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
CONTIKI_SOURCEFILES +=  pb_decode.c pb_encode.c settings.pb.c readings.pb.c sampling-sensors.c  filenames.c config.c sampler.c filenames.c poster.c ms_webserver.c
PROJECTDIRS += ../../examples/ipv6/rpl-border-router
PROJECT_SOURCEFILES += httpd-simple.c

include $(CONTIKI)/Makefile.include

$(CONTIKI)/tools/tunslip6:	$(CONTIKI)/tools/tunslip6.c
	(cd $(CONTIKI)/tools && $(MAKE) tunslip6)

connect-router:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 aaaa::1/64

connect-router-cooja:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -a 127.0.0.1 aaaa::1/64

install: z1-sampler.upload

CUSTOM_RULE_LINK=1 
%.$(TARGET): %.co $(PROJECT_OBJECTFILES) $(PROJECT_LIBRARIES) contiki-$(TARGET).a $(LD) $(LDFLAGS) $(TARGET_STARTFILES) ${filter-out %.a,$^} ${filter %.a,$^} $(TARGET_LIBFILES) -o $@ -lm
