# LeForum Documentation
## Known Issues
#### All the data sent by server may not be more than 0xFFFF bytes long. If it's bigger, then it will be cut down to the 0xFFFF
The problem is in [server/server.c](/src/server/server.c), `handle_client()`. Currently it works in the way GET->SEND->GET->SEND->... Therefore, it is not supported to send more than 0xFFFF bytes (`send()` limit).

#### All the `*_load()` functions will crash the program if the file they read is not valid
Affected source files: [server/server.c](/src/server/server.c), [server/core/forum.c](/src/server/core/forum.c).

#### Several functions, constants, variables are named using different styles
Doesn't affect program workflow, but readability is dead

#### Overcommenting
Yes.
