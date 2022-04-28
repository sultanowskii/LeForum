#ifndef LEFORUM_LIB_SECURITY_H_
#define LEFORUM_LIB_SECURITY_H_

#include <stdint.h>
#include <stdio.h>

#include "lib/status.h"

#define NULLPTR_PREVENT(ptr, LESTATUS) if (ptr == nullptr) {return LESTATUS;}

/**
 * @brief Generates random string of given size and writes to the place where str points.
 * 
 * @param str Pointer, which will point to the resulting string 
 * @param size String size 
 * @return LESTATUS_OK on succes 
 */
status_t            rand_string(char *str, size_t size);

/**
 * @brief Returns random unsigned 8-byte number. 
 * 
 * @return Random unsigned long long 
 */
uint64_t           rand_uint64_t();

#endif