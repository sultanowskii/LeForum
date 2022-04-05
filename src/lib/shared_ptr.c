#include "lib/shared_ptr.h"

SharedPtr * sharedptr_create(void *data, void (*destruct)(void *)) {
	SharedPtr     *new_sharedptr;

	if (data == nullptr || destruct == nullptr) {
		return LESTATUS_NPTR;
	}

	new_sharedptr = malloc(sizeof(SharedPtr));

	new_sharedptr->ref_count = malloc(sizeof(int64_t));
	*new_sharedptr->ref_count = 1;

	new_sharedptr->data = data;

	new_sharedptr->destruct = destruct;


	return new_sharedptr;
}

SharedPtr * sharedptr_add(SharedPtr * sharedptr) {
	SharedPtr     *new_sharedptr;

	if (sharedptr == nullptr) {
		return LESTATUS_NPTR;
	}

	new_sharedptr = malloc(sizeof(SharedPtr));
	new_sharedptr->ref_count = sharedptr->ref_count;
	(*new_sharedptr->ref_count)++;
	new_sharedptr->data = sharedptr->data;
	new_sharedptr->destruct = sharedptr->destruct;

	return new_sharedptr;
}

status_t sharedptr_delete(SharedPtr * sharedptr) {
	if (sharedptr == nullptr) {
		return LESTATUS_NPTR;
	}
	
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