/**
 *  \file new 1
 *  \brief Brief
 */


/* Includes --------------------------------------------------------------------------------*/
#include "circ_buffer.h"	// header file
#include "bsp_config.h"		// bsp_CONFIG Header file (for ATOMIC_BLOCK)

/* Private declarations --------------------------------------------------------------------*/

/* Initialization functions ----------------------------------------------------------------*/
/**
  * @brief	Initialize the circular buffer.
  * 		@note	Externally allocated buffer
  * @param	CB: pointer to a CIRCBUFF_Struct.
  * @param	size: desired size of the circular buffer.
  * @retval	none.
  */
void CIRCBUFF_Struct_Init(CIRCBUFF_Struct * CB, char * buffer, uint32_t size)
{
	CB->size = size;
	CB->itemCount = 0;
	CB->head = 0;
	CB->tail = 0;
	CB->buffer =  buffer;
}
/**
  * @brief	Initialize the circular buffer.
  * 		@note	Internally dynamically allocated buffer
  * @param	CB: pointer to a CIRCBUFF_Struct.
  * @param	size: desired size of the circular buffer.
  * @retval	1 if initialized, 0 otherwise.
  */
/*
uint8_t CIRCBUFF_Struct_InitAllocate(CIRCBUFF_Struct * CB, uint32_t size)
{
	CB->size = size;
	CB->itemCount = 0;
	CB->head = 0;
	CB->tail = 0;
	CB->buffer =  (char*)(malloc(size * sizeof(char)));
	if (CB->buffer == NULL) return 0;
	return 1;
}
*/
void CIRCBUFF_Flush(CIRCBUFF_Struct * CB)
{
	CB->itemCount = 0;
	CB->head = 0;
	CB->tail = 0;
}
/* Management functions --------------------------------------------------------------------*/
/**
  * @brief	Verify whether the buffer is not empty.
  * @param	CB: pointer to a CIRCBUFF_Struct.
  * @retval	1 if at least 1 more item, 0 if empty.
  */
uint8_t CIRCBUFF_HasItem(CIRCBUFF_Struct * CB)
{
	if (CB->itemCount>0) return 1;
	return 0;
}
/**
  * @brief	Get the number of items inside the buffer.
  * @param	CB: pointer to a CIRCBUFF_Struct.
  * @retval	number of items.
  */
uint32_t CIRCBUFF_Items(CIRCBUFF_Struct * CB)
{
	return CB->itemCount;
}

/**
  * @brief	Insert an item in the buffer.
  * @param	CB: pointer to a CIRCBUFF_Struct.
  * @param	item: The item to be inserted in the buffer.
  * @retval	1 if item has been inserted, 0 otherwise.
  */
uint8_t CIRCBUFF_PutItem(CIRCBUFF_Struct * CB, char item)
{
	// Do not insert the item if buffer is full
	if ((CB->itemCount)>=(CB->size)) return 0;
	// Put the item atomically, not to corrupt the buffer
	ATOMIC_BLOCK(
		/* Save the item in the buffer */
		CB->buffer[CB->head] = item;
		/* Update head position */
		CB->head = (CB->head+1)%CB->size;
		/* Refresh the item count */
		CB->itemCount++;
	);
	// Return ok result
	return 1;
}
/**
  * @brief	Insert items in the buffer.
  * @param	CB: pointer to a CIRCBUFF_Struct.
  * @param	items: The items to be inserted in the buffer.
  * @param	length: The number of items to insert in the buffer.
  * @retval	the number of item inserted.
  */
uint32_t CIRCBUFF_PutItems(CIRCBUFF_Struct * CB, char * items, uint32_t length)
{
	uint32_t inserted = 0;
	// Do it atomically, not to corrupt the buffer
	ATOMIC_BLOCK(
		for (inserted=0; inserted<length; inserted++)
		{
			/* Break for if buffer is full */
			if ((CB->itemCount)>=(CB->size)) break;
			/* Save the item in the buffer */
			CB->buffer[CB->head] = items[inserted];
			/* Update head position */
			CB->head = (CB->head+1)%CB->size;
			/* Refresh the item count */
			CB->itemCount++;
		}
	);
	// Return the number of inserted items
	return inserted;
}
/**
  * @brief	Get the next item of the buffer.
  * @param	CB: pointer to a CIRCBUFF_Struct.
  * @retval	the item.
  */
char CIRCBUFF_GetItem(CIRCBUFF_Struct * CB)
{
	char tempItem;
	// Get the item atomically, not to corrupt the buffer
	ATOMIC_BLOCK(
		/* Get the item from the buffer */
		tempItem = CB->buffer[CB->tail];
		/* Update tail position */
		CB->tail = (CB->tail+1)%CB->size;
		/* Refresh the item count */
		CB->itemCount--;
	);
	// Return the item
	return tempItem;
}

/**
  * @brief	Move every item from a CBOri to CBDest.
  * @param	CBOri: pointer to a CIRCBUFF_Struct (origin).
  * @param	CBDest: pointer to a CIRCBUFF_Struct (destination).
  * @param	limit: max number of items to be moved.
  * @retval	the number of moved items.
  */
uint32_t CIRCBUFF_MoveItems(CIRCBUFF_Struct * CBOri, CIRCBUFF_Struct * CBDest, uint32_t limit)
{
	uint32_t moved = 0;
	// Do it atomically, not to corrupt the buffer
	ATOMIC_BLOCK(
		/* Till when there are items and dest is not full*/
		while ((CBOri->itemCount>0) && ((CBDest->itemCount)<(CBDest->size)) && (moved<limit))
		{
		/* Get Item from CBOri */
			/* Get the item from the buffer at the next head position */
			char tempItem = CBOri->buffer[CBOri->tail];
			/* Update head position */
			CBOri->tail = (CBOri->tail+1)%CBOri->size;
			/* Refresh the item count */
			CBOri->itemCount--;
		/* Put Item into CBDest */
			/* Save the item in the buffer */
			CBDest->buffer[CBDest->head] = tempItem;
			/* Update head position */
			CBDest->head = (CBDest->head+1)%CBDest->size;
			/* Refresh the item count */
			CBDest->itemCount++;
		/* Update moved */
			moved++;
		}
	);
	// Return the number of inserted items
	return moved;
}
