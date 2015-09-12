all: z1-nothing
TARGET = z1-feshie

CONTIKI=../../

CONTIKI_WITH_IPV6=1
CFLAGS += -DUIP_CONF_IPV6=1

# Disable human readable process names - they're stored in .data
CFLAGS += -DPROCESS_CONF_NO_PROCESS_NAMES=1

SMALL=1

CONTIKI_SOURCEFILES +=

include $(CONTIKI)/Makefile.include

install: z1-feshie.upload

CUSTOM_RULE_LINK=1
%.$(TARGET): %.co $(PROJECT_OBJECTFILES) $(PROJECT_LIBRARIES) contiki-$(TARGET).a $(LD) $(LDFLAGS) $(TARGET_STARTFILES) ${filter-out %.a,$^} ${filter %.a,$^} $(TARGET_LIBFILES) -o $@ -lm
