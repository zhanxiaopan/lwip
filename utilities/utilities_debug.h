/*
 * utilities_debug.h
 *
 *  Created on: Sep 11, 2017
 *      Author: CNTHXIE
 */

#ifndef UTILITIES_UTILITIES_DEBUG_H_
#define UTILITIES_UTILITIES_DEBUG_H_

#include <stdio.h>

#define UTILS_ASSERT(expr) do                                                 \
                     {                                                        \
                         if(!(expr))                                          \
                         {                                                    \
                             while(1);                                        \
                         }                                                    \
                     }                                                        \
                     while(0)

#endif /* UTILITIES_UTILITIES_DEBUG_H_ */
