/**
 *  @file utilities.c
 *  @author wenlong li
 *  @version v0.2
 *  @brief implement some general functions.
 *          - digital IO edge detection.
 *          - 2-order low-pass filter
 *          - ddd
 *  
 *  @date 2016.8.8 v0.1 created with customized softwared based DIDO processing.
 *  @20161027	updated to v0.2
 *  			add the new features to process data stream, mainly for moving average/sum calculation.
 *  @20161031	fix the bug caused by the misuse of double*.
 *  			correct the error in average calculation.
 */

#include "utilities.h"

/* --------------- START of DIDO edge process block --------------- */
/**
 *  @brief Return the edge as unsiged value.
 *  @return None
 *  @details None
 */
uint8_t dio_detect_edge (DIO_BLOCK_T *io)
{
    return (io->edge);
}


/**
 *  @brief return signed edge value
 *  @param [in] io IO to process
 *  @return s_edge
 *  @details Details
 */
int8_t dio_detect_s_edge (DIO_BLOCK_T *io)
{
    return (io->s_edge);
}


/**
 *  @brief Brief
 *  @param [in] io Parameter_Description
 *  @return Return_Description
 *  @details Details
 */
void dio_block_process (DIO_BLOCK_T *io)
{
    io->previous_state = io->current_state;
    io->current_state = *(io->src);
    
    io->edge = (io->current_state ^ io->previous_state)? (2 - io->current_state) : 0;
    io->s_edge = io->edge ? 5 - io->edge * 3 : 0;   
}
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

/**
 *  @brief Init the obj of data stream.
 *  
 *  @param [in] p_stream data_stream to init
 *  @param [in] d_buf    pointer to data buf, normal a elsewhere defined data array.
 *  @param [in] d_src    pointer to data src.
 *  @param [in] d_size   number of the element.
 *  @return None
 *  
 *  @details None
 */
void data_stream_init(DATA_STREAM_T *p_stream, double *d_buf, double *d_src, uint16_t d_size) {
	p_stream->pBuf = d_buf;
	p_stream->pData = d_src;
	p_stream->size = d_size;
	p_stream->pNew = d_buf;

	p_stream->average = 0;
	p_stream->sum = 0;

	if (p_stream->pBuf == 0 || p_stream->pData == 0 || p_stream->size == 0 || p_stream->pNew == 0) {
		// todo report error.
	}
}

/**
 *  @brief Process the data stream object with
 *   - calculate sum
 *   - calculate average
 *   - update data buf and pNew.
 *  
 *  @param [in] p_stream data_stream object to process
 *  @return None
 *  
 *  @details None
 *  	p_stream->pNew ++ will add the lenght of double (8) to the pointer, NOT 1!
 *  	p_stream->pNew - p_stream->pBuf will also be in the unit of lenght of "double".
 *
 */
void data_stream_process(DATA_STREAM_T *p_stream) {
	// calculate the sum and average.
	p_stream->sum += (*(p_stream->pData) - *(p_stream->pNew));
	p_stream->average = p_stream->sum / p_stream->size;

	// update data, and pNew.
	*(p_stream->pNew) = *(p_stream->pData);
	p_stream->pNew ++;
	if ((p_stream->pNew - p_stream->pBuf) >= (p_stream->size)) {
		p_stream->pNew = p_stream->pBuf;
	}
}
/* --------------- END of data_stream operation block --------------- */
