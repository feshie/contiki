#define PROTBUF_OPCODE_ECHO 0x00
#define PROTBUF_OPCODE_LIST 0x01
#define PROTBUF_OPCODE_GET_DATA 0x02
#define PROTBUF_OPCODE_SET_GAIN 0x03
#define PROTBUF_OPCODE_RESPONSE 0xFF

#define PROTBUF_MASTER_ADDR 0x00

#define PROTBUF_MAX_MESSAGE_LENGTH 128
#define PROTOBUF_RETRIES 3

void protobuf_process_message(uint8_t *buf, uint8_t bytes);
void protobuf_send_message(uint8_t addr, uint8_t opcode, uint8_t *payload, int8_t payload_length);

void protobuf_handler_set_writeb(void (*wb)(unsigned char c));

void protobuf_register_process_callback(struct process *p, process_event_t ev);

void protobuf_init(void);


typedef struct{
  uint8_t length;
  uint8_t *data;
}protobuf_data_t;

