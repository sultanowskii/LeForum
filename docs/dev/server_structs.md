# LeForum Developer Documentation
## Server inner structures
Currently methods aren't included here, so please refer to header files for explanation.

### LeThread
**Declared in**: [src/server/forum.h](/src/server/core/forum.h)

**Description**: Contains information about a thing which widely known as Fourm Thread - discussion, tree of messages

**Fields**: 
`id` - Unique ID
`first_message_id` - ID of the first message. Message IDs are unique in one LeThread, they can collapse through different LeThreads
`next_message_id` - ID of the next message that will be created
`topic` - LeThread topic (aka head message)
`messages` - Queue containing [message](#LeMessage) history
`author` - Pointer to the [LeAuthor](#LeAuthor) of this thread

---

### LeMessage
**Declared in**: [src/server/forum.h](/src/server/core/forum.h)

**Description**: Contains information about message. Related with specific LeThread

**Fields**:
`id` - Unique ID
`text` - Pointer to  the text
`by_lethread_author` - Bool that shows if this LeMessage is posted by author of the thread or not
`lethread` - Back pointer to the LeThread where this message is posted

---

### LeAuthor
**Declared in**: [src/server/forum.h](/src/server/core/forum.h)

**Description**: Contains information about author

**Fields**:
`id` - Unique ID
`token` - Random string of fixed size used in API calls (for message creation, for example) to set **by_lethread_author** to `TRUE`

---

### LeCommand
**Declared in**: [src/server/query.h](/src/server/core/query.h)

**Description**: Contains information about "command" - query processor and its trigger

**Fields**:
`name` - String that represents request name. When client sends a query, the first bytes are always a name (basically, "command trigger")
`process` - Function that parses query (basically, "command executor")

---

### LeCommandResult
**Declared in**: [src/server/query.h](/src/server/core/query.h)

**Description**: Contains result of the query processor

**Fields**:
`data` - Pointer to the data that has to be sent to client as a response.
`size` - Size of the data.
`status` - Inner status of the function execution

---

### LeClientInfo
**Declared in**: [src/sever/server.h](/src/server/server.h)

**Description**: Contains information about connected client

**Fields**:
`fd` - Socket FD used for communication with client
`addr` - `sockaddr_in` structure (from `<sys/socket.c>`), contains client connection information
`addr_size` - size of `addr`