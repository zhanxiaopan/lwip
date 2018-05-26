#ifndef __USER_FIFO_H /* __USER_FIFO_H */
#define __USER_FIFO_H

#define USE_UTILITIES

#include <stdint.h>
#include <string.h>

#define FIFO_DATABUF_SIZE_INBYTE (8192)

typedef struct FIFO_BUFFER_T {
    uint8_t data_buf[FIFO_DATABUF_SIZE_INBYTE];
    uint16_t pos_in_buf;   // current data pos in the buffer, start from 0, end in MEASURE_DATA_BUFFER_SIZE-1. data_buf[pos_in_buf] is a free pos.
    uint16_t data_start_ptr; // where valid data start in the buf. buffered data are the ones from data_buf[data_start_ptr]  to data_buf[pos_in_buf-1].
} FIFO_BUFFER_T;

void fifo_write (FIFO_BUFFER_T* buf, char *src, uint16_t len);
int fifo_get_clear (FIFO_BUFFER_T* buf, char* dest);

#endif /* __USER_FIFO_H */
