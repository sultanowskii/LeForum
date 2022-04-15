#pragma once

/**
 * @brief Returns maximum between two given arguments. 
 * 
 */
#define MAX(x, y)            (((x) > (y)) ? (x) : (y))

/**
 * @brief Returns minimum between two given arguments. 
 * 
 */
#define MIN(x, y)            (((x) < (y)) ? (x) : (y))

/**
 * @brief Returns increased by one x. If it becomes more than max, min is returned.  
 * 
 */
#define cyclic_inc(x, min, max) x = ((x + 1) <= (max)) ? (x + 1) : (min)

/**
 * @brief Returns decreased by one x. If it becomes less than min, max is returned.  
 * 
 */
#define cyclic_dec(x, min, max) x = ((x - 1) >= (min)) ? (x - 1) : (max)