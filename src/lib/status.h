#pragma once

#include <stdint.h>

#define LESTATUS_OK          0  /* Ok */
#define LESTATUS_CLIB        -1 /* C Library Call Error */
#define LESTATUS_NSFD        -2 /* No Such File or Directory */
#define LESTATUS_EXST        -3 /* Already Exists */
#define LESTATUS_NFND        -4 /* Not found */
#define LESTATUS_ISYN        -5 /* Invalid Syntax */
#define LESTATUS_IDAT        -6 /* Invalid Data */
#define LESTATUS_NPTR        -7 /* Null pointer */

typedef int8_t status_t;