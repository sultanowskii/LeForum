# LeForum Documentation
## Known Issues
#### No sanity checks of the data size both in server and client
It means we cann allocate chunk of any size.

#### All the `load()` functions will cause segfault if the file they read is not valid
Affected source files: [server/server.c](/src/server/server.c), [lib/forum.c](/src/lib/forum.c).
