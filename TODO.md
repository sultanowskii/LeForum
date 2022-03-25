# TODO

### Permanent reminders
- Check all the malloc() <-> free() logic
- ptr == NULL checks
- Check if all the `status_t`s are appropriate
- Tests for most of the features

### Mini-tasks
- [x] Clean up lib/query.c
- [ ] Tests for query.c with python + socket (compile + run server, then check explicit queries)
- [ ] Make lethread_get_by_id work with lethread_queue (then remove free(lethread) from query.c)
- [x] Query manager for LeMessage. Current problem is that all the "*_save()" functions take only LeThread, and lemessage_save() takes 2 arguments. How should they be stored?
- [ ] Already existing lethreads loader on a startup

### Global features
#### Server
- [ ] Query processing
- [ ] Argument handling
#### Client
- [ ] Query processing
- [ ] Argument handling
- [ ] TUI
- [ ] Tokens logic
- [ ] Customisation