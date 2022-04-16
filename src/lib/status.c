#include "lib/status.h"

inline const char *get_lestatus_string_repr(status_t status) {
	switch (status) {
		case LESTATUS_OK:   return "OK";
		case LESTATUS_CLIB: return "CLIB";
		case LESTATUS_NSFD: return "NSFD";
		case LESTATUS_EXST: return "EXST";
		case LESTATUS_NFND: return "NFND";
		case LESTATUS_ISYN: return "ISYN";
		case LESTATUS_IDAT: return "IDAT";
		case LESTATUS_NPTR: return "NPTR";
		default: "";
	}
}