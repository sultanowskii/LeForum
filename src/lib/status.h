#pragma once

#include <stdint.h>

#define LESTATUS_OK         0 /* Ok */
#define LESTATUS_NFND       1 /* Not found */
#define LESTATUS_CLIB      -1 /* C Library Call Error */
#define LESTATUS_NSFD      -2 /* No Such File or Directory */
#define LESTATUS_EXST      -3 /* Already Exists */

typedef int8_t status_t;