#ifndef LEFORUM_LIB_SECURITY_H_
#define LEFORUM_LIB_SECURITY_H_

#include <stdint.h>
#include <stdio.h>

#include "lib/status.h"

#define NULLPTR_PREVENT(ptr, LESTATUS) if (ptr == nullptr) {return LESTATUS;}

/**
 * @brief Returns random string of given size 
 * 
 * @param size String size 
 * @return Pointer to random string 
 */
char *rand_string(size_t size);

/**
 * @brief Returns random unsigned 8-byte number. 
 * 
 * @return Random unsigned long long 
 */
uint64_t rand_uint64_t();

#endif