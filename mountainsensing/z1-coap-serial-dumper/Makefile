all: serial-dumper
TARGET = z1-feshie

CONTIKI=../../
PROTOBUF:=../protocol-buffers/
NANOPB:=$(PROTOBUF)tools/nanopb/

CONTIKI_WITH_IPV6=1
CFLAGS += -DUIP_CONF_IPV6=1
CFLAGS += -DREST=coap_rest_implementation

SMALL=1

PROJECTDIRS += $(NANOPB) $(PROTOBUF)c/ ../common/

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
CONTIKI_SOURCEFILES += pb_decode.c pb_encode.c pb_common.c all.c store.c

include $(CONTIKI)/Makefile.include

install: serial-dumper.upload

CUSTOM_RULE_LINK=1
%.$(TARGET): %.co $(PROJECT_OBJECTFILES) $(PROJECT_LIBRARIES) contiki-$(TARGET).a $(LD) $(LDFLAGS) $(TARGET_STARTFILES) ${filter-out %.a,$^} ${filter %.a,$^} $(TARGET_LIBFILES) -o $@ -lm
