all: kl25z-nothing
TARGET = frdm-kl25z-rf

CONTIKI=../..

CONTIKI_WITH_IPV6=1
CFLAGS += -DUIP_CONF_IPV6=1

# Disable human readable process names - they're stored in .data
CFLAGS += -DPROCESS_CONF_NO_PROCESS_NAMES=1

SMALL=1

CONTIKI_SOURCEFILES += 

include $(CONTIKI)/Makefile.include
