/*
 *            Copyright (c) 2002-2013 by Real Time Automation, Inc.
 *
 *  This software is copyrighted by and is the sole property of
 *  Real Time Automation, Inc. (RTA).  All rights, title, ownership, 
 *  or other interests in the software remain the property of RTA.  
 *  This software may only be used in accordance with the corresponding
 *  license agreement.  Any unauthorized use, duplication, transmission,
 *  distribution, or disclosure of this software is expressly forbidden.
 *
 *  This Copyright notice MAY NOT be removed or modified without prior
 *  written consent of RTA.
 *
 *  RTA reserves the right to modify this software without notice.
 *
 *  Real Time Automation
 *  150 S. Sunny Slope Road            USA 262.439.4999
 *  Suite 130                          www.rtaautomation.com
 *  Brookfield, WI 53005               software@rtaautomation.com
 *
 *************************************************************************
 *
 *    Version Date: 05NOV2013
 *         Version: 2.37
 *    Conformed To: EtherNet/IP Protocol Conformance Test A-10 (28-AUG-2012)
 *     Module Name: eips_tag.c
 *          Author: Jamin D. Wendorf (jwendorf@rtaautomation.com)
 *        Language: Ansi C
 * Compile Options: N/A
 * Compile defines: N/A
 *       Libraries: N/A
 *    Link Options: N/A
 *
 * Description.
 * =======================================================================
 * This file contains the optional tag read/write server code (act like a
 * Logix processor.
 *
 */

/* ---------------------------- */
/*      INCLUDE FILES           */
/* ---------------------------- */

#include "eips_system.h"

/* used for debug */
#undef  __RTA_FILE__
#define __RTA_FILE__ "eips_tag.c"

#if defined (EIPS_CIP_READ_WRITE_DATA_LOGIX) || defined (EIPS_CIP_READ_WRITE_DATA_LOGIX_ADVANCED)

/* ---------------------------- */
/* FUNCTION PROTOTYPES          */
/* ---------------------------- */

/* ---------------------------- */
/* GLOBAL VARIABLES		        */
/* ---------------------------- */

/* ---------------------------- */
/* MISCELLANEOUS  		        */
/* ---------------------------- */
uint8 temp_data_buffer[500];

/**/
/* ******************************************************************** */
/*	   	 	            GLOBAL FUNCTIONS		                        */
/* ******************************************************************** */
/* ====================================================================
Function:   eips_tag_process_msg
Parameters: Pointer to Message Router Request structure
            Pointer to Message Router Response structure
Returns:    SUCCESS - Use response in "response" structure
            FAILURE - Use default error "Object does not exist"

If the user supports any tag (ASCII name) objects, parse the message,
validate the passed request and build a response message in the passed
response strucute.  If the passed object isn't supported, return
FAILURE and don't fill in the passed response structure.
======================================================================= */
#ifdef EIPS_CIP_READ_WRITE_DATA_LOGIX
uint8 eips_tag_process_msg (EIPS_USER_MSGRTR_REQ_FMT *request, EIPS_USER_MSGRTR_RSP_FMT *response)
{
    EIPS_TAG    local_tag;
    uint8       path_seg, path_val;
    uint16      i, j, ix, number_elements, expected_size;
    int16       rc=-1;
    uint16      data_type = 0;
    uint16      temp_data_ptr_size = 0;
    uint8       *temp_data_ptr   = NULL;
    int8        *temp_int8_ptr  = NULL;
    int16       *temp_int16_ptr = NULL;
    int32       *temp_int32_ptr = NULL;
    float       *temp_float_ptr  = NULL;

    if(!request || !response)
        return(FAILURE);

    /* initialize variables */
    memset(&local_tag, 0, sizeof(local_tag));

    /* parse the path */
    for(i=0; i<request->path_size; i++)
    {
        path_seg = EIPS_LO(request->path[i]);
        path_val = EIPS_HI(request->path[i]);

        /* The only valid order is Class, Inst, Attr */
        switch (path_seg)
        {
            /* ANSI Extended Symbol Segment (1 byte to follow - len if bytes), then ASCII string */
            case ANSIEXT_SYMSEG:
                if(i != 0)
                    local_tag.elements_used++;

                if(local_tag.elements_used < EIPS_TAG_MAX_NUM_ELEMENT_LEVELS)
                {
                    local_tag.element[local_tag.elements_used].str_len = path_val;    
                    for(j=0; j<local_tag.element[local_tag.elements_used].str_len && i<request->path_size; j+=2)
                    {
                        i++;
                        rta_PutLitEndian16(request->path[i], (uint8*)&local_tag.element[local_tag.elements_used].str[j]);
                    }
                }
                /* error, we received too many levels */
                else
                {
                    response->gen_stat = ERR_PATHSEGMENT;
                    return(SUCCESS); /* use our error code */
                }
                break;

            /* array index (stored in a member path) */
            case LOGSEG_8BITMEMBER:
                if(local_tag.element[local_tag.elements_used].array_ix_used < EIPS_TAG_MAX_NUM_ARRAY_LEVELS)
                {
                    local_tag.element[local_tag.elements_used].array_ix[local_tag.element[local_tag.elements_used].array_ix_used] = path_val;
                    local_tag.element[local_tag.elements_used].array_ix_used++;
                }
                break;

            case LOGSEG_16BITMEMBER:
                if(local_tag.element[local_tag.elements_used].array_ix_used < EIPS_TAG_MAX_NUM_ARRAY_LEVELS)
                {
                    i++;
                    local_tag.element[local_tag.elements_used].array_ix[local_tag.element[local_tag.elements_used].array_ix_used] = request->path[i];
                    local_tag.element[local_tag.elements_used].array_ix_used++;
                }
                break;

            default: /* we see a path we don't understand */
                return(FAILURE);
        };
    }

    if(request->path_size)
        local_tag.elements_used++;
        
    /* parse based on the class */
    switch(request->service)
    {
        /* **************************************************** */
        /*                  READ REQUEST                        */
        /* **************************************************** */
        case RA_SC_RD_TAG_REQ_REQ:
            /* we need only two bytes of data */
            if(request->req_data_size != 2)
            {
                response->gen_stat = ERR_TOOMUCH_DATA;
                return(SUCCESS); /* use our error code */
            }

            /* store the number_elements */
            number_elements = rta_GetLitEndian16(request->req_data);

            /* validate the tag, get the data pointer from the user */
            rc = eips_usertag_get_read_data (&local_tag, number_elements, &data_type, temp_data_buffer, sizeof(temp_data_buffer));
            switch(rc)
            {
                case EIPS_TAGERR_SUCCESS: 
                    /* success, so continue processing */
                    break;

                case EIPS_TAGERR_TAG_NOT_FOUND: 
                    response->gen_stat = ERR_PATHSEGMENT;
                    return(SUCCESS); /* use our error code */

                case EIPS_TAGERR_OFFSET:
                case EIPS_TAGERR_DATA_TYPE:
                    response->gen_stat = 0xFF;
                    response->ext_stat_size = 1;
                    response->ext_stat[0] = rc; /* internal error */
                    return(SUCCESS); /* use our error code */

                default:
                    response->gen_stat = ERR_VENDOR_SPECIFIC;
                    response->ext_stat_size = 1;
                    response->ext_stat[0] = rc; /* internal error */
                    return(SUCCESS); /* use our error code */
            };

            response->gen_stat = 0;
            response->rsp_data_size = 0;
            ix = 0;

            /* store the data type in the response (an error will clear out) */
            rta_PutLitEndian16(data_type, response->rsp_data+response->rsp_data_size);
            response->rsp_data_size+=2;

            switch(data_type)
            {
                /* one byte of data per SINT */
                case EIPS_CL_DATATYPE_SINT:
                    temp_int8_ptr = (int8 *)temp_data_buffer;
                    rta_ByteMove (response->rsp_data+response->rsp_data_size, temp_int8_ptr, number_elements);
                    response->rsp_data_size += number_elements;
                    break;

                /* two bytes of data per INT */
                case EIPS_CL_DATATYPE_INT:
                    temp_int16_ptr = (int16 *)temp_data_buffer;
                    for(i=0; i<number_elements; i++)
                    {   
                        rta_PutLitEndian16(temp_int16_ptr[ix++], response->rsp_data+response->rsp_data_size);
                        response->rsp_data_size+=2;
                    }
                    break;

                /* four bytes of data per DINT */
                case EIPS_CL_DATATYPE_DINT:
                    temp_int32_ptr = (int32 *)temp_data_buffer;
                    for(i=0; i<number_elements; i++)
                    {   
                        rta_PutLitEndian32(temp_int32_ptr[ix++], response->rsp_data+response->rsp_data_size);
                        response->rsp_data_size+=4;
                    }
                    break;

                /* four bytes of data per REAL (float) */
                case EIPS_CL_DATATYPE_REAL:
                    temp_float_ptr = (float *)temp_data_buffer;
                    for(i=0; i<number_elements; i++)
                    {   
                        rta_PutLitEndianFloat(temp_float_ptr[ix++], response->rsp_data+response->rsp_data_size);
                        response->rsp_data_size+=4;
                    }
                    break;

                /* one byte of data 0 if the bit is 0 and FF is the bit is 1 */
                case EIPS_CL_DATATYPE_BOOL:
                /* four bytes of data per BIT ARRAY (32 bits) */
                case EIPS_CL_DATATYPE_BITARRAY:
                default:
                    return(FAILURE);
            };
            return(SUCCESS);

        /* **************************************************** */
        /*                  WRITE REQUEST                       */
        /* **************************************************** */
        case RA_SC_WR_TAG_REQ_REQ:
            /* we need at least two more bytes for the data type */
            if(request->req_data_size < 4)
            {
                response->gen_stat = ERR_INSUFF_DATA;
                return(SUCCESS); /* use our error code */
            }

            /* store the data type */
            data_type = rta_GetLitEndian16(request->req_data);

            /* store the number_elements */
            number_elements = rta_GetLitEndian16(request->req_data+2);

            /* store the data pointer */
            temp_data_ptr = request->req_data+4;
            temp_data_ptr_size = (request->req_data_size-4);

            /* convert data to native format */
            switch(data_type)
            {
                /* one byte of data per SINT */
                case EIPS_CL_DATATYPE_SINT:
                    expected_size = number_elements;
                    /* validate the size */
                    if(expected_size != temp_data_ptr_size)
                    {
                        if(temp_data_ptr_size < expected_size)
                            response->gen_stat = ERR_INSUFF_DATA;
                        else
                            response->gen_stat = ERR_TOOMUCH_DATA;
                        return(SUCCESS); /* use our error code */
                    }

                    rta_ByteMove (temp_data_buffer, temp_data_ptr, expected_size);
                    break;

                /* two bytes of data per INT */
                case EIPS_CL_DATATYPE_INT:
                    expected_size = (number_elements*2);
                    /* validate the size */
                    if(expected_size != temp_data_ptr_size)
                    {
                        if(temp_data_ptr_size < expected_size)
                            response->gen_stat = ERR_INSUFF_DATA;
                        else
                            response->gen_stat = ERR_TOOMUCH_DATA;
                        return(SUCCESS); /* use our error code */
                    }

                    temp_int16_ptr = (int16 *)temp_data_buffer;
                    for(i=0; i<number_elements; i++)
                    {   
                        temp_int16_ptr[i] = rta_GetLitEndian16(temp_data_ptr);
                        temp_data_ptr+=2;
                    }
                    break;

                /* four bytes of data per DINT */
                case EIPS_CL_DATATYPE_DINT:
                    expected_size = (number_elements*4);
                    /* validate the size */
                    if(expected_size != temp_data_ptr_size)
                    {
                        if(temp_data_ptr_size < expected_size)
                            response->gen_stat = ERR_INSUFF_DATA;
                        else
                            response->gen_stat = ERR_TOOMUCH_DATA;
                        return(SUCCESS); /* use our error code */
                    }

                    temp_int32_ptr = (int32 *)temp_data_buffer;
                    for(i=0; i<number_elements; i++)
                    {   
                        temp_int32_ptr[i] = rta_GetLitEndian32(temp_data_ptr);
                        temp_data_ptr+=4;
                    }
                    break;

                /* four bytes of data per REAL (float) */
                case EIPS_CL_DATATYPE_REAL:
                    expected_size = (number_elements*4);
                    /* validate the size */
                    if(expected_size != temp_data_ptr_size)
                    {
                        if(temp_data_ptr_size < expected_size)
                            response->gen_stat = ERR_INSUFF_DATA;
                        else
                            response->gen_stat = ERR_TOOMUCH_DATA;
                        return(SUCCESS); /* use our error code */
                    }

                    temp_float_ptr = (float *)temp_data_buffer;
                    for(i=0; i<number_elements; i++)
                    {   
                        temp_float_ptr[i] = rta_GetLitEndianFloat(temp_data_ptr);
                        temp_data_ptr+=4;
                    }
                    break;

                /* one byte of data 0 if the bit is 0 and FF is the bit is 1 */
                case EIPS_CL_DATATYPE_BOOL:
                /* four bytes of data per BIT ARRAY (32 bits) */
                case EIPS_CL_DATATYPE_BITARRAY:
                default:
                    return(FAILURE);
            };

            /* validate the tag, get the data pointer from the user */
            rc = eips_usertag_set_write_data (&local_tag, number_elements, data_type, temp_data_buffer, temp_data_ptr_size);
            switch(rc)
            {
                case EIPS_TAGERR_SUCCESS: 
                    /* success, so continue processing */
                    break;

                case EIPS_TAGERR_TAG_NOT_FOUND: 
                    response->gen_stat = ERR_PATHSEGMENT;
                    return(SUCCESS); /* use our error code */

                case EIPS_TAGERR_OFFSET:
                case EIPS_TAGERR_DATA_TYPE:
                    response->gen_stat = 0xFF;
                    response->ext_stat_size = 1;
                    response->ext_stat[0] = rc; /* internal error */
                    return(SUCCESS); /* use our error code */

                default:
                    response->gen_stat = ERR_VENDOR_SPECIFIC;
                    response->ext_stat_size = 1;
                    response->ext_stat[0] = rc; /* internal error */
                    return(SUCCESS); /* use our error code */
            };

            response->gen_stat = 0;
            response->rsp_data_size = 0;
            ix = 0;

            /* store the data type in the response (an error will clear out) */
            rta_PutLitEndian16(data_type, response->rsp_data+response->rsp_data_size);
            response->rsp_data_size+=2;
            return(SUCCESS);
    };

    return(FAILURE);
}
#endif

/* ====================================================================
Function:   eips_tag_process_msg_adv
Parameters: Pointer to Message Router Request structure
            Pointer to Message Router Response structure
Returns:    SUCCESS - Use response in "response" structure
            FAILURE - Use default error "Object does not exist"

If the user supports any tag (ASCII name) objects, parse the message,
validate the passed request and build a response message in the passed
response strucute.  If the passed object isn't supported, return
FAILURE and don't fill in the passed response structure.
======================================================================= */
#ifdef EIPS_CIP_READ_WRITE_DATA_LOGIX_ADVANCED
uint8 eips_tag_process_msg_adv (EIPS_USER_MSGRTR_REQ_FMT *request, EIPS_USER_MSGRTR_RSP_FMT *response)
{
    EIPS_TAG    local_tag;
    uint8       path_seg, path_val;
    uint16      i, j, ix, number_elements;
    int16       rc=-1;
    uint16      data_type = 0;
    uint16      struct_handle = 0;
#ifdef EIPS_CIP_READ_WRITE_DATA_LOGIX_ADVANCED
    uint32      offset = 0;
#endif
    uint16      temp_data_ptr_size = 0;
    uint8       *temp_data_ptr   = NULL;

    if(!request || !response)
        return(FAILURE);

    /* initialize variables */
    memset(&local_tag, 0, sizeof(local_tag));

    /* parse the path */
    for(i=0; i<request->path_size; i++)
    {
        path_seg = EIPS_LO(request->path[i]);
        path_val = EIPS_HI(request->path[i]);

        /* The only valid order is Class, Inst, Attr */
        switch (path_seg)
        {
            /* ANSI Extended Symbol Segment (1 byte to follow - len if bytes), then ASCII string */
            case ANSIEXT_SYMSEG:
                if(i != 0)
                    local_tag.elements_used++;

                if(local_tag.elements_used < EIPS_TAG_MAX_NUM_ELEMENT_LEVELS)
                {
                    local_tag.element[local_tag.elements_used].str_len = path_val;    
                    for(j=0; j<local_tag.element[local_tag.elements_used].str_len && i<request->path_size; j+=2)
                    {
                        i++;
                        rta_PutLitEndian16(request->path[i], (uint8*)&local_tag.element[local_tag.elements_used].str[j]);
                    }
                }
                /* error, we received too many levels */
                else
                {
                    response->gen_stat = ERR_PATHSEGMENT;
                    return(SUCCESS); /* use our error code */
                }
                break;

            /* array index (stored in a member path) */
            case LOGSEG_8BITMEMBER:
                if(local_tag.element[local_tag.elements_used].array_ix_used < EIPS_TAG_MAX_NUM_ARRAY_LEVELS)
                {
                    local_tag.element[local_tag.elements_used].array_ix[local_tag.element[local_tag.elements_used].array_ix_used] = path_val;
                    local_tag.element[local_tag.elements_used].array_ix_used++;
                }
                break;

            case LOGSEG_16BITMEMBER:
                if(local_tag.element[local_tag.elements_used].array_ix_used < EIPS_TAG_MAX_NUM_ARRAY_LEVELS)
                {
                    i++;
                    local_tag.element[local_tag.elements_used].array_ix[local_tag.element[local_tag.elements_used].array_ix_used] = request->path[i];
                    local_tag.element[local_tag.elements_used].array_ix_used++;
                }
                break;

            default: /* we see a path we don't understand */
                return(FAILURE);
        };
    }

    if(request->path_size)
        local_tag.elements_used++;
        
    /* parse based on the class */
    switch(request->service)
    {
        /* **************************************************** */
        /*                  READ REQUEST                        */
        /* **************************************************** */
        case RA_SC_RD_TAG_REQ_REQ:
            /* we need only two bytes of data */
            if(request->req_data_size != 2)
            {
                response->gen_stat = ERR_TOOMUCH_DATA;
                return(SUCCESS); /* use our error code */
            }

            /* store the number_elements */
            number_elements = rta_GetLitEndian16(request->req_data);

            /* validate the tag, get the data pointer from the user */
            temp_data_ptr_size = sizeof(temp_data_buffer);
            rc = eips_usertag_get_read_data_formatted (&local_tag, number_elements, &data_type, &struct_handle, temp_data_buffer, &temp_data_ptr_size);
            switch(rc)
            {
                case EIPS_TAGERR_SUCCESS: 
                    /* success, so continue processing */
                    break;

                case EIPS_TAGERR_TAG_NOT_FOUND: 
                    response->gen_stat = ERR_PATHSEGMENT;
                    return(SUCCESS); /* use our error code */

                case EIPS_TAGERR_OFFSET:
                case EIPS_TAGERR_DATA_TYPE:
                    response->gen_stat = 0xFF;
                    response->ext_stat_size = 1;
                    response->ext_stat[0] = rc; /* internal error */
                    return(SUCCESS); /* use our error code */

                default:
                    response->gen_stat = ERR_VENDOR_SPECIFIC;
                    response->ext_stat_size = 1;
                    response->ext_stat[0] = rc; /* internal error */
                    return(SUCCESS); /* use our error code */
            };

            response->gen_stat = 0;
            response->rsp_data_size = 0;
            ix = 0;

            /* store the data type in the response (an error will clear out) */
            rta_PutLitEndian16(data_type, response->rsp_data+response->rsp_data_size);
            response->rsp_data_size+=2;

            /* if we are a structure, add the handle */
            if(data_type == EIPS_CL_DATATYPE_STRUCTURE)
            {
                rta_PutLitEndian16(struct_handle, response->rsp_data+response->rsp_data_size);
                response->rsp_data_size+=2;
            }

            /* will the response fit? */
            if((response->rsp_data_size + temp_data_ptr_size) > sizeof(response->rsp_data))
            {
                /* truncate the data */
                temp_data_ptr_size = (sizeof(response->rsp_data) - response->rsp_data_size);
                response->gen_stat = ERR_PARTIALXFER; /* not enough space, fit what we can */
            }

            /* store the data */
            rta_ByteMove (response->rsp_data+response->rsp_data_size, temp_data_buffer, temp_data_ptr_size);
            response->rsp_data_size += temp_data_ptr_size;
            return(SUCCESS);

        /* **************************************************** */
        /*                  WRITE REQUEST                       */
        /* **************************************************** */
        case RA_SC_WR_TAG_REQ_REQ:
            /* store the data pointer */
            temp_data_ptr = request->req_data;
            temp_data_ptr_size = request->req_data_size;

            /* we need at least two more bytes for the data type */
            if(temp_data_ptr_size < 4)
            {
                response->gen_stat = ERR_INSUFF_DATA;
                return(SUCCESS); /* use our error code */
            }

            /* store the data type */
            data_type = rta_GetLitEndian16(temp_data_ptr);
            temp_data_ptr+=2; temp_data_ptr_size-=2;

            /* store the structure handle (if we are a structure) */
            if(data_type == EIPS_CL_DATATYPE_STRUCTURE) 
            {
                if(temp_data_ptr_size >= 2) 
                {
                    struct_handle = rta_GetLitEndian16(temp_data_ptr);
                    temp_data_ptr+=2; temp_data_ptr_size-=2;
                }
                else
                {
                    response->gen_stat = ERR_INSUFF_DATA;
                    return(SUCCESS); /* use our error code */
                }
            }

            /* store the number_elements */
            if(temp_data_ptr_size >= 2)
            {
                number_elements = rta_GetLitEndian16(temp_data_ptr);
                temp_data_ptr+=2; temp_data_ptr_size-=2;
            }
            else
            {
                response->gen_stat = ERR_INSUFF_DATA;
                return(SUCCESS); /* use our error code */
            }

            /* validate the tag, pass the data pointer to the user */
            rc = eips_usertag_set_write_data_formatted (&local_tag, number_elements, data_type, struct_handle, temp_data_ptr, temp_data_ptr_size);
            switch(rc)
            {
                case EIPS_TAGERR_SUCCESS: 
                    /* success, so continue processing */
                    break;

                case EIPS_TAGERR_TAG_NOT_FOUND: 
                    response->gen_stat = ERR_PATHSEGMENT;
                    return(SUCCESS); /* use our error code */

                case EIPS_TAGERR_OFFSET:
                case EIPS_TAGERR_DATA_TYPE:
                    response->gen_stat = 0xFF;
                    response->ext_stat_size = 1;
                    response->ext_stat[0] = rc; /* internal error */
                    return(SUCCESS); /* use our error code */

                default:
                    response->gen_stat = ERR_VENDOR_SPECIFIC;
                    response->ext_stat_size = 1;
                    response->ext_stat[0] = rc; /* internal error */
                    return(SUCCESS); /* use our error code */
            };

            response->gen_stat = 0;
            response->rsp_data_size = 0;
            return(SUCCESS);

        /* **************************************************** */
        /*                  FRAG READ REQUEST                   */
        /* **************************************************** */
        case RA_SC_RD_FRAG_TAG_REQ_REQ:
            /* we need only six bytes of data */
            if(request->req_data_size != 6)
            {
                response->gen_stat = ERR_TOOMUCH_DATA;
                return(SUCCESS); /* use our error code */
            }

            /* store the number_elements */
            number_elements = rta_GetLitEndian16(request->req_data);

            /* store the offset */
            offset = rta_GetLitEndian32(request->req_data+2);

            /* validate the tag, get the data pointer from the user */
            temp_data_ptr_size = RTA_MIN(EIPS_FRAG_TAG_MAX_DATA_ON_READ, sizeof(temp_data_buffer));
            rc = eips_usertag_get_frag_read_data_formatted (&local_tag, number_elements, offset, &data_type, &struct_handle, temp_data_buffer, &temp_data_ptr_size);
            switch(rc)
            {
                case EIPS_TAGERR_SUCCESS: 
                    /* success, so continue processing */            
                    response->gen_stat = ERR_SUCCESS;
                    break;

                case EIPS_TAGERR_PARTIAL_XFER:
                    /* success, but only a partial transfer, so store the error and continue processing */
                    response->gen_stat = ERR_PARTIALXFER;
                    break;

                case EIPS_TAGERR_TAG_NOT_FOUND: 
                    response->gen_stat = ERR_PATHSEGMENT;
                    return(SUCCESS); /* use our error code */

                case EIPS_TAGERR_OFFSET:
                case EIPS_TAGERR_DATA_TYPE:
                    response->gen_stat = 0xFF;
                    response->ext_stat_size = 1;
                    response->ext_stat[0] = rc; /* internal error */
                    return(SUCCESS); /* use our error code */

                default:
                    response->gen_stat = ERR_VENDOR_SPECIFIC;
                    response->ext_stat_size = 1;
                    response->ext_stat[0] = rc; /* internal error */
                    return(SUCCESS); /* use our error code */
            };

            response->rsp_data_size = 0;
            ix = 0;

            /* store the data type in the response (an error will clear out) */
            rta_PutLitEndian16(data_type, response->rsp_data+response->rsp_data_size);
            response->rsp_data_size+=2;

            /* if we are a structure, add the handle */
            if(data_type == EIPS_CL_DATATYPE_STRUCTURE)
            {
                rta_PutLitEndian16(struct_handle, response->rsp_data+response->rsp_data_size);
                response->rsp_data_size+=2;
            }

            /* will the response fit? */
            if((response->rsp_data_size + temp_data_ptr_size) > sizeof(response->rsp_data))
            {
                /* truncate the data */
                temp_data_ptr_size = (sizeof(response->rsp_data) - response->rsp_data_size);
                response->gen_stat = ERR_PARTIALXFER; /* not enough space, fit what we can */
            }

            /* store the data */
            rta_ByteMove (response->rsp_data+response->rsp_data_size, temp_data_buffer, temp_data_ptr_size);
            response->rsp_data_size += temp_data_ptr_size;
            return(SUCCESS);

        /* **************************************************** */
        /*                  FRAG WRITE REQUEST                  */
        /* **************************************************** */
        case RA_SC_WR_FRAG_TAG_REQ_REQ:
            /* store the data pointer */
            temp_data_ptr = request->req_data;
            temp_data_ptr_size = request->req_data_size;

            /* we need at least eight more bytes */
            if(temp_data_ptr_size < 8)
            {
                response->gen_stat = ERR_INSUFF_DATA;
                return(SUCCESS); /* use our error code */
            }

            /* store the data type */
            data_type = rta_GetLitEndian16(temp_data_ptr);
            temp_data_ptr+=2; temp_data_ptr_size-=2;

            /* store the structure handle (if we are a structure) */
            if(data_type == EIPS_CL_DATATYPE_STRUCTURE) 
            {
                if(temp_data_ptr_size >= 2) 
                {
                    struct_handle = rta_GetLitEndian16(temp_data_ptr);
                    temp_data_ptr+=2; temp_data_ptr_size-=2;
                }
                else
                {
                    response->gen_stat = ERR_INSUFF_DATA;
                    return(SUCCESS); /* use our error code */
                }
            }

            /* store the number_elements */
            if(temp_data_ptr_size >= 2)
            {
                number_elements = rta_GetLitEndian16(temp_data_ptr);
                temp_data_ptr+=2; temp_data_ptr_size-=2;
            }
            else
            {
                response->gen_stat = ERR_INSUFF_DATA;
                return(SUCCESS); /* use our error code */
            }

            /* store the offset */
            if(temp_data_ptr_size >= 4)
            {
                offset = rta_GetLitEndian32(temp_data_ptr);
                temp_data_ptr+=4; temp_data_ptr_size-=4;
            }
            else
            {
                response->gen_stat = ERR_INSUFF_DATA;
                return(SUCCESS); /* use our error code */
            }

            /* validate the tag, pass the data pointer to the user */
            rc = eips_usertag_set_frag_write_data_formatted (&local_tag, number_elements, offset, data_type, struct_handle, temp_data_ptr, temp_data_ptr_size);
            switch(rc)
            {
                case EIPS_TAGERR_SUCCESS: 
                    /* success, so continue processing */
                    break;

                case EIPS_TAGERR_TAG_NOT_FOUND: 
                    response->gen_stat = ERR_PATHSEGMENT;
                    return(SUCCESS); /* use our error code */

                case EIPS_TAGERR_OFFSET:
                case EIPS_TAGERR_DATA_TYPE:
                    response->gen_stat = 0xFF;
                    response->ext_stat_size = 1;
                    response->ext_stat[0] = rc; /* internal error */
                    return(SUCCESS); /* use our error code */

                default:
                    response->gen_stat = ERR_VENDOR_SPECIFIC;
                    response->ext_stat_size = 1;
                    response->ext_stat[0] = rc; /* internal error */
                    return(SUCCESS); /* use our error code */
            };

            response->gen_stat = 0;
            response->rsp_data_size = 0;
            return(SUCCESS);
    };

    return(FAILURE);
}
#endif

/* ====================================================================
Function:   eips_tag_reassemble_tagname 
Parameters: pointer to tag structure
            pointer to store reassembled tag name
            max len of tag name
Returns:    <0 error, else size of new tag name
======================================================================= */
int16 eips_tag_reassemble_tagname (EIPS_TAG *tag, char *str, uint16 max_len_str)
{
    char reassembled_tag[500];
    int16 rc,i,j;

    /* validate the pointers */
    if(!tag || !str || max_len_str==0)
        return(-1);

    /* zero out the pointers */
    memset(reassembled_tag, 0, sizeof(reassembled_tag));
    memset(str, 0, max_len_str);

    /* fill in the tag names and array indicies */
    for(i=0; i<tag->elements_used; i++)    
    {
        /* first time, just store the initial name */       
        if(i==0)
            sprintf(reassembled_tag, "%s", tag->element[i].str);
        /* after first time, append new string as a structure element */
        else
            sprintf(reassembled_tag, "%s.%s",reassembled_tag, tag->element[i].str);

        /* append array indicies (if needed) */
        for(j=0; j<tag->element[i].array_ix_used; j++)
        {
            sprintf(reassembled_tag, "%s[%d]",reassembled_tag, tag->element[i].array_ix[j]);        
        }
    }

    /* make sure the whole string fits */
    rc = RTA_MIN(max_len_str, (uint16)strlen(reassembled_tag));

    /* store the string */
    memcpy(str, reassembled_tag, rc);

    /* return the string size */
    return(rc);
}

/**/
/* ******************************************************************** */
/*	   	 	            LOCAL FUNCTIONS  		                        */
/* ******************************************************************** */
/* ====================================================================
Function:   
Parameters: 
Returns:    
======================================================================= */

#endif /* defined (EIPS_CIP_READ_WRITE_DATA_LOGIX) || defined (EIPS_CIP_READ_WRITE_DATA_LOGIX_ADVANCED) */

/* *********** */
/* END OF FILE */
/* *********** */
