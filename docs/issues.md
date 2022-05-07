# LeForum Documentation
## Known Issues
#### All the data sent by server may not be more than 0xFFFF bytes long. If it's bigger, then it will be cut down to the 0xFFFF
The problem is in [server/server.c](/src/server/server.c), `handle_client()`. Currently it works in the way GET->SEND->GET->SEND->... Therefore, it is not supported to send more than 0xFFFF bytes (`send()` limit).

#### All the `load()` functions will cause segfault if the file they read is not valid
Affected source files: [server/server.c](/src/server/server.c), [lib/forum.c](/src/lib/forum.c).
