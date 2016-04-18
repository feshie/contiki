DEFINES+=PROJECT_CONF_H=\"project-conf.h\"
CONTIKI_PROJECT = remote-nothing
TARGET = zoul
BOARD = remote

CONTIKI=../../

CONTIKI_WITH_IPV6=1
CFLAGS += -DUIP_CONF_IPV6=1

# Disable human readable process names - they're stored in .data
# CFLAGS += -DPROCESS_CONF_NO_PROCESS_NAMES=1

all: $(CONTIKI_PROJECT)

CONTIKI_SOURCEFILES +=

include $(CONTIKI)/Makefile.include

