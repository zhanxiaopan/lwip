/**
 *  \file new 1
 *  \brief Brief
 */

#ifndef __CIRC_BUFFER_H_
#define __CIRC_BUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes --------------------------------------------------------------------------------*/
#include <stdlib.h>		// Common types
#include <stdint.h>		// Integer types

/* Configurations --------------------------------------------------------------------------*/

/* Definitions -----------------------------------------------------------------------------*/
/**
 * @brief	Circular Buffer Structure
 */
typedef volatile struct
{
	uint32_t size;					// Size of the buffer
	uint32_t itemCount;				// Number of item in the buffer
	uint32_t head;					// Head of the buffer
	uint32_t tail;					// Tail of the buffer
	char * buffer;
} CIRCBUFF_Struct;

/* Functions -------------------------------------------------------------------------------*/
void		CIRCBUFF_Struct_Init(CIRCBUFF_Struct * CB, char * buffer, uint32_t size);
//uint8_t		CIRCBUFF_Struct_InitAllocate(CIRCBUFF_Struct * CB, uint32_t size);
void		CIRCBUFF_Flush(CIRCBUFF_Struct * CB);
uint8_t		CIRCBUFF_HasItem(CIRCBUFF_Struct * CB);
uint32_t	CIRCBUFF_Items(CIRCBUFF_Struct * CB);
uint8_t		CIRCBUFF_PutItem(CIRCBUFF_Struct * CB, char item);
uint32_t	CIRCBUFF_PutItems(CIRCBUFF_Struct * CB, char * items, uint32_t length);
char		CIRCBUFF_GetItem(CIRCBUFF_Struct * CB);
uint32_t	CIRCBUFF_MoveItems(CIRCBUFF_Struct * CBOri, CIRCBUFF_Struct * CBDest, uint32_t limit);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CIRC_BUFFER_H_ */
