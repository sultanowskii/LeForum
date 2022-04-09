# LeForum Developer Documentation
## General inner structure methods
### Queue
#### `struct Queue * queue_create(status_t (*destruct)(void *))`
**Description**: 
Creates a new Queue object.

**Parameters**:
`destruct` - Callback that safely deletes one object stored in a queue. Is called for each object stored in a queue

**Return value**:
Pointer to created Queue

#### `status_t queue_delete(struct Queue *queue)`
**Description**: 
Safely deletes the Queue and all the elements. Use this _only_ if you created queue by using `queue_create()`.

**Parameters**:
`queue` - Pointer to Queue to delete

**Return value**:
LESTATUS_OK on success

#### `status_t queue_push(struct Queue *queue, void *data, size_t size)`
**Description**: 
Adds a new element to the end of the Queue.

**Parameters**:
`queue` - Pointer to Queue where a new element will be placed
`data`  - Pointer to data to place
`size` -  Size of the data

**Return value**:
LESTATUS_OK on success

#### `void * queue_pop(struct Queue *queue)`
**Description**: 
Retrieves the first element and deletes it from the Queue.

**Parameters**:
`queue` - Pointer to Queue to pop element from

**Return value**:
Pointer to the popped data

#### `bool_t queue_is_empty(struct Queue *queue)`
**Description**: 
Checks whether queue is empty or not.

**Parameters**:
`queue` - Pointer to Queue to check

**Return value**:
TRUE if queue is empty, otherwise ELSE

---

### QueueNode
No methods, because it's sub-struct of the [Queue](#Queue)

---

### SharedPtr
#### `SharedPtr * sharedptr_create(void *data, status_t (*destruct)(void *))`
**Description**: 
Creates SharedPtr (the first one, aka origin).

**Parameters**:
`data`- Pointer to the data. Keep in mind that this parameter is stored, so don't `free()` it yourself
`destruct` - `data` destructor callback

**Return value**:
Pointer to the SharedPtr object

#### `sharedptr_add(SharedPtr * sharedptr)`
**Description**: 
Creates another instance of SharedPtr, increments reference counter of the give sharedptr.

**Parameters**:
`sharedptr` - Pointer to the origin SharedPtr

**Return value**:
Pointer to the copy of SharedPtr

#### `status_t sharedptr_delete(SharedPtr * sharedptr)`
**Description**: 
Safely deletes the SharedPtr instance. Decrements reference counter. If it is equal to 0, then deletes the data stored in it (by calling `destruct())` the user-made callback).

**Parameters**:
`sharedptr` - Pointer to SharedPtr

**Return value**:
LESTATUS_OK on success
