#include "lib/status.h"

const char *SREPR_LESTATUS[] = { 
	[LESTATUS_OK]   =    "OK",
	[LESTATUS_CLIB] =    "CLIB",
	[LESTATUS_NSFD] =    "NSFD",
	[LESTATUS_EXST] =    "EXST",
	[LESTATUS_NFND] =    "NFND",
	[LESTATUS_ISYN] =    "ISYN",
	[LESTATUS_IDAT] =    "IDAT",
	[LESTATUS_NPTR] =    "NPTR",
};

inline const char *get_lestatus_string_repr(status_t status) {
	return SREPR_LESTATUS[status];
}