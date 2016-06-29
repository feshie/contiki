all: z1-sniffer
TARGET = z1-wais

CONTIKI=../../

DEFINES+=PROJECT_CONF_H=\"project-conf.h\"

CONTIKI_WITH_IPV6=1
CFLAGS += -DUIP_CONF_IPV6=1

# Disable human readable process names - they're stored in .data
CFLAGS += -DPROCESS_CONF_NO_PROCESS_NAMES=1

PROJECT_SOURCEFILES += slip-bridge.c

PROJECT_SOURCEFILES+= sniffer-rdc-driver.c

SMALL=1

CONTIKI_SOURCEFILES +=

include $(CONTIKI)/Makefile.include

install: z1-sniffer.upload

slip2pcap: slip2pcap.c
	@gcc -o slip2pcap slip2pcap.c

sniff: slip2pcap
	@sudo echo # ask for the password first (make login may require it)
	@make login | ./slip2pcap | wireshark -k -i -

sniff-raw: slip2pcap
	@sudo echo # ask for the password first (make login may require it)
	@make login | ./slip2pcap


CUSTOM_RULE_LINK=1
%.$(TARGET): %.co $(PROJECT_OBJECTFILES) $(PROJECT_LIBRARIES) contiki-$(TARGET).a $(LD) $(LDFLAGS) $(TARGET_STARTFILES) ${filter-out %.a,$^} ${filter %.a,$^} $(TARGET_LIBFILES) -o $@ -lm
