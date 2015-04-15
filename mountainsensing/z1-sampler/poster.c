
#include "poster.h"

int
handle_connection(char *data_buffer, uint8_t data_length, uint8_t *http_status, struct psock *p, uint8_t *psock_buffer)
{
    char content_length[8], tmpstr_handle[50];
    PPRINT("Data length = %d\n", data_length);
    if(data_length > 0){
        PSOCK_BEGIN(p);
        PPRINT("begun\n");
        itoa(data_length, content_length, 10);
        strcpy(tmpstr_handle, "POST / HTTP/1.0\r\nContent-Length: ");
        strcat(tmpstr_handle, content_length);
        strcat(tmpstr_handle, "\r\n\r\n");
        PPRINT("Prepared string\n");
    
        
        PSOCK_SEND_STR(p, tmpstr_handle);
        PPRINT("String sent\n");
        printf("*******************************************************************\n");
        PSOCK_SEND(p, data_buffer, data_length);
        printf("*******************************************************************\n");
        PPRINT("Data sent wating for status\n");
        while(1) {
            PPRINT("W");
            PSOCK_READTO(p, '\n');
            if(strncmp(psock_buffer, "HTTP/", 5) == 0){   
                // Status line
                *http_status = atoi((const char *)psock_buffer + 9);
            }
        }
        PPRINT("about to end\n");
        PSOCK_END(p);
    }else{
        printf("Invalid data length in handle connection\n" );
        PSOCK_BEGIN(p);
        PSOCK_END(p);
    }
}

/*---------------------------------------------------------------------------*/

//returns number of bytes loaded

uint8_t 
load_file(char *data_buffer, char *filename)
{
    int fd;
    uint8_t data_length;
#ifdef SPI_LOCKING
    LPRINT("LOCK: load file\n");
    NETSTACK_MAC.off(0);
    cc1120_arch_interrupt_disable();
    CC1120_LOCK_SPI();  
#endif
    fd = cfs_open(filename, CFS_READ);
    if(fd >= 0){
        data_length = cfs_read(fd, data_buffer, DATA_BUFFER_LENGTH);
        cfs_close(fd);
        PPRINT("[LOAD] Read %d bytes from %s\n", data_length, filename);
    }else{
        PPRINT("[LOAD] ERROR: CAN'T READ FILE { %s }\n", filename);
        data_length = 0;
    }
#ifdef SPI_LOCKING
    LPRINT("UNLOCK: load file\n");
    CC1120_RELEASE_SPI();
    cc1120_arch_interrupt_enable();
    NETSTACK_MAC.on();
#endif
    return data_length;
}