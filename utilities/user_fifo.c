/**
 *  @file user_fifo.c
 *  @author wenlong.li
 *  @version v0.1
 *  @brief software-based fifo which provide basic write and read(get) interface.
 *          - define FIFO_BUFFER_T variable where you need.
 *          - call fifo_write to write data into fifo.
 *          - call fifo_get_clear to fetch data from fifo.
 *          - we assume that fifo_get_clear could be interrupted by fifo_write in user application. (ok to send out later but record should be real-time.)
 *  @History
		- 20160816 firstly created with basic function ready.
		- 20160818 fix the possible "overlap" in fifo_write.
 *  @
 */


#include "user_fifo.h"

void fifo_write (FIFO_BUFFER_T* buf, char *src, uint16_t len) {
    // check the remaining available space in the buffer
    // should refuse the writing in case of no enough free buf space and report this error.
	// todo
    
    // write into the "circular" buf.
    if (len > (FIFO_DATABUF_SIZE_INBYTE - buf->pos_in_buf)) {
        // 1st, write until the end of buffer.
        // wrap back to start point of the buffer.
        memcpy (buf->data_buf + buf->pos_in_buf, src, FIFO_DATABUF_SIZE_INBYTE - buf->pos_in_buf);
        memcpy (buf->data_buf, src + FIFO_DATABUF_SIZE_INBYTE - buf->pos_in_buf, len - (FIFO_DATABUF_SIZE_INBYTE - buf->pos_in_buf));
        buf->pos_in_buf = len - (FIFO_DATABUF_SIZE_INBYTE - buf->pos_in_buf);
    }
    else {
        // no need to go back to original point of the fifo buffer. write directly.
        memcpy(buf->data_buf + buf->pos_in_buf, src, len);
        buf->pos_in_buf = (buf->pos_in_buf + len) % FIFO_DATABUF_SIZE_INBYTE;        
    }
}

/**
 *  @brief get all available data from the fifo buffer and clear it!
 *  @param [in] buf  fifo buf
 *  @param [in] dest destination to read
 *  @return return the bytes number gotten.
 *  @details Details
 */
int fifo_get_clear (FIFO_BUFFER_T* buf, char* dest) {
    // we use temp var cp and sp here to avoid any chaos since this routine could be interrupted by fifo_write().
    uint16_t cp = 0, sp = 0;
    int data_len = 0;
    
    cp = buf->pos_in_buf;
    sp = buf->data_start_ptr;
    
    if (sp < cp) {
        memcpy(dest, buf->data_buf+sp, cp-sp);
        data_len = cp - sp;
    }
    else if (sp > cp) {
        memcpy(dest, buf->data_buf+sp, FIFO_DATABUF_SIZE_INBYTE-sp);
        memcpy(dest + FIFO_DATABUF_SIZE_INBYTE-sp, buf->data_buf, cp);
        data_len = FIFO_DATABUF_SIZE_INBYTE - (sp -cp);
    }
    else {
        // this means there is actually no data buffered since last 'get'.
        // currently do nothing.
    	data_len = 0;
    }
    
    //memset(buf->data_buf, 0, FIFO_DATABUF_SIZE_INBYTE);
    buf->data_start_ptr = cp;

    return data_len;
}

#if 0
// only as reference to show how to write data into fifo.
// preferred that the written data is in text (string).
// application dependant
#define MEASURE_DATA_UNIT_SIZE (30)

void measure_write2fifo () {
    // depend on the format of the data you want to write.
    static char temp_str_buf[MEASURE_DATA_UNIT_SIZE];
    int temp_str_len;
    
    memest(temp_str_buf, 0, MEASURE_DATA_UNIT_SIZE);    
    temp_str_len = snprintf (temp_str_buf, MEASURE_DATA_UNIT_SIZE, "%d %d %.2f \n", flowrate, fs1_t, fs2_t);
    fifo_write(fifobuf1, temp_str_buf, temp_str_len);  
}
#endif
