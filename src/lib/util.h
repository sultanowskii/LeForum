#ifndef LEFORUM_LIB_UTIL_H_
#define LEFORUM_LIB_UTIL_H_

#include <stdio.h>
#include <stdlib.h>

#define newline() printf("\n")

/**
 * @brief Returns maximum between two given arguments. 
 * 
 */
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

/**
 * @brief Returns minimum between two given arguments. 
 * 
 */
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

/**
 * @brief Returns increased by one x. 
 * If it becomes more than max, min is returned.  
 * 
 */
#define cyclic_inc(x, min, max) x = ((x + 1) <= (max)) ? (x + 1) : (min)

/**
 * @brief Returns decreased by one x. 
 * If it becomes less than min, max is returned. 
 * 
 */
#define cyclic_dec(x, min, max) x = ((x - 1) >= (min)) ? (x - 1) : (max)

/**
 * @brief Suppresses "unused parameter" warning. 
 * 
 */
#define UNUSED(arg) ((void)arg)

/**
 * @brief Naive data with size container. 
 *
 */
struct LeData {
	void   *data;
	size_t  size;
};
typedef struct LeData LeData;

/**
 * @brief fgets() wrapper that sets '\0' instead of '\n'. 
 * 
 * @param s Data pointer where input will be stored
 * @param n Max amounts to read 
 * @param fd File descriptor to read from 
 * @return Size of read data on success. 
 * LESTATUS_CLIB is retured if fgets() failed 
 */
size_t s_fgets(char *s, size_t n, FILE* fd);

/**
 * @brief s_fgets() wrapper that reads at least _min and at max _max bytes. 
 * 
 * @param s Data pointer where input will be stored 
 * @param _min Min number of bytes to read (includinlgly) 
 * @param _max Max number of bytes to read (includinlgly) 
 * @param fd File descriptor to read from 
 * @return Size of read data on success. LESTATUS_IDAT if s_fgets() failed 
 */
size_t s_fgets_range(char *s, size_t _min, size_t _max, FILE* fd);

#endif
