# LeForum Developer Documentation
## Tests
The only test you should use now is [test-server-queries](/src/test/test-server-queries.py).

### test-server-queries
**Description**:
It basically tests all the API queries. Before running it, you should run server first (on port 7431, the default one).

Note that this file has some [requirements](/src/test/requirements.txt)

**Basic usage**:
```bash
python3 test/test-server-queries.py
```

**Possible arguments**:
`NOBASIC` - Doesn't run GTHR and CTHR tests
`NOFIND` - Doesn't run FTHR tests
`NOMETA` - Doesn't run META test
`INTERACTIVE` - Simulates working client - you can send queries by yourself. 

**INTERACTIVE mode command syntax**:
```
CTHR [topic]
```

```
GTHR [lethread_id]
```

```
FTHR [part of topic to find]
```

```
CMSG [lethread_id] [text]
```

```
META
```