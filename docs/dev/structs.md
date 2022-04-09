# LeForum Developer Documentation
## General inner structures
### Queue
**Declared in**: [src/lib/queue.h](/src/lib/queue.h)

**Description**: Queue structure (widely known structure, you can find information in the Internet), implemeted as linked list. Supports to store only one kind of data in each node

**Fields**:
`size` - number of [nodes](#QueueNode) in the queue
`first` - first node (aka HEAD)
`last` - last node (aka TAIL)
`destruct` - callback that destructs the data stored in queue nodes (free() securely)

### QueueNode
**Declared in**: [src/lib/queue.h](/src/lib/queue.h)

**Description**: Queue node, is one "element" of the queue

**Fields**:
`data` - pointer to the data itself
`next` - pointer to the next node

### SharedPtr
**Declared in**: [src/lib/shared_ptr.h](/src/lib/shared_ptr.h)

**Description**: Shared pointer (widely known structure, you can find information in the Internet) implemetation

**Fields**:
`ref_count` - number of references to the `data`. In other words, number of sharedptr copies
`data` - pointer to the data
`destruct` - callback that destructs the `data` (free() securely)