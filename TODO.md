## TODO

- [x] Clean up lib/query.c
- [] Check all the malloc() <-> free() logic
- [] ptr == NULL checks
- [] Tests for query.c with python + socket
- [] Check if all the `status_t`s are appropriate
- [] Tests for most of the features
- [] decide about lethread_get_by_id finally (if returned from the list, then remove free() in the end of queries)