#ifndef __UTILITIES_H
#define __UTILITIES_H

#define USE_UTILITIES

#include <stdint.h>

/* --------------- START of DIDO edge process block --------------- */
typedef struct DIO_BLOCK_T
{
    // ptr to src signal.
    uint8_t *src;

    // state buffer
    uint8_t current_state;
    uint8_t previous_state;

    // edge output - falling/rising edge determined output.
    // e.g. falling edge output 0 and keep until a rising edge.
    uint8_t edge;       // edge signal, 1: rising edge. 2: falling edge, 0: no edge.
    int8_t s_edge;      // signed edge signal, -1: falling edge, 1: rising edge. 0: no edge.
} DIO_BLOCK_T;

uint8_t dio_detect_edge (DIO_BLOCK_T *io);
int8_t dio_detect_s_edge (DIO_BLOCK_T *io);
void dio_block_process (DIO_BLOCK_T *io);
/* --------------- END of DIDO edge process block --------------- */


/* --------------- START of data_stream operation block --------------- */
/*
 * =================== Application Notes ===================
 * Data stream is widely used in signal analysis.
 * Assume that data are sampled with fixed period.
 * Currently supported data type is only double.
 * Currently requiring externally defined data array like: double data_array[100];
 * Currently supported operations are
 *  - data buf - replace oldest data with newest one.
 *  - moving sum
 *  - moving average
 * */

typedef struct DATA_STREAM_T {
	double *pBuf;
	double *pData;
	uint16_t size;	// the numbler of elements in the buf. each element of double type should occupy 4 bytes in ANSI C.

	double *pNew;	// pNew is where we need to put in new data. so it also where the oldest data is.

	double sum;
	double average;

} DATA_STREAM_T;

void data_stream_init(DATA_STREAM_T *p_stream, double *d_buf, double *d_src, uint16_t d_size);
void data_stream_process(DATA_STREAM_T *p_stream);
/* --------------- END of data_stream operation block --------------- */
#endif

