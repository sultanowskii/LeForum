# TODO

### Permanent reminders
- Check all the malloc() <-> free() logic
- ptr == NULL checks
- Check if all the `status_t`s are appropriate
- Tests for most of the features

### Mini-tasks
- [x] Clean up lib/query.c
- [x] Make lethread_get_by_id work with lethread_queue (then remove free(lethread) from query.c)
- [x] Query manager for LeMessage. Current problem is that all the "*_save()" functions take only LeThread, and lemessage_save() takes 2 arguments. How should they be stored?
- [x] Already existing lethreads loader on a startup
- [x] Thread creation query
- [x] Find by topic query
- [x] Test find by topic query
- [x] Meta-query (max topic size, max message size, number of lethreads)
- [x] Metadata: Max topic size, max message size, etc.
- [x] Safe ID generation in lethread creation (to prevent collisions)
- [x] Safe multithread counter interface (using locks, idk)
- [x] Metadata file interface (it has to contain number of lethreads at least) + loading next_lethread_id_value
- [x] shared_ptr for server/server.c and server/query.c to prevent double free on cleanup (now it happens because different pointers in different queues may point to one object)
- [x] Destructor callback for Queue (for pop() and delete() operations)
- [x] sharedptr_add() in s_X_create() - to make sharedptrs work correctly
- [x] ptr == nullptr checks in server/server.c
- [x] Removing disconnected clients from corresponding queue
- [x] NULLing pointers on free(): Either by passing void** arguments to delete_X() functions or manually.
- [x] Test for messages
- [ ] Data size sanity check

### Global features
#### Server
- [x] Query processing
- [x] Arguments
- [x] Documentation
#### Client
- [x] Query processing
- [x] Arguments
- [x] Tokens logic
- [ ] Customisation
- [ ] Settings