#include "lib/shared_ptr.h"

#include <stdlib.h>

#include "lib/constants.h"
#include "lib/security.h"
#include "lib/status.h"

status_t sharedptr_create(void *data, status_t (*destruct)(void *), SharedPtr **sharedptr) {
	SharedPtr *new_sharedptr;

	NULLPTR_PREVENT(data, -LESTATUS_NPTR)
	NULLPTR_PREVENT(destruct, -LESTATUS_NPTR)

	new_sharedptr = malloc(sizeof(*new_sharedptr));

	new_sharedptr->ref_count = malloc(sizeof(new_sharedptr->ref_count));
	*new_sharedptr->ref_count = 1;

	new_sharedptr->data = data;

	new_sharedptr->destruct = destruct;

	if (sharedptr != nullptr) 
		*sharedptr = new_sharedptr;
	
	new_sharedptr = nullptr;

	return LESTATUS_OK;
}

SharedPtr *sharedptr_add(SharedPtr *sharedptr) {
	SharedPtr *new_sharedptr;

	if (sharedptr == nullptr)
		return nullptr;

	new_sharedptr = malloc(sizeof(*new_sharedptr));
	new_sharedptr->ref_count = sharedptr->ref_count;
	(*new_sharedptr->ref_count)++;
	new_sharedptr->data = sharedptr->data;
	new_sharedptr->destruct = sharedptr->destruct;

	return new_sharedptr;
}

status_t sharedptr_delete(SharedPtr *sharedptr) {
	NULLPTR_PREVENT(sharedptr, -LESTATUS_NPTR)

	(*sharedptr->ref_count)--;

	if (sharedptr->ref_count != nullptr && sharedptr->destruct != nullptr && *sharedptr->ref_count <= 0) {
		sharedptr->destruct(sharedptr->data);

		sharedptr->data = nullptr;

		sharedptr->destruct = nullptr;

		free(sharedptr->ref_count);
		sharedptr->ref_count = nullptr;
	}

	free(sharedptr);
	sharedptr = nullptr;

	return LESTATUS_OK;
}