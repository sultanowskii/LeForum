# LeForum Developer Documentation
## API
Communication between server and client is an infinite SEND -> RECV -> REPEAT conversation. If client wants to request some endpoint, it sends query to server. The response will come in the nearest client RECV. If client doesn't want anything from server, then it sends LIVE query.

Before sending each query/reponse, size of it is sent first (as 8-byte number).

To read the whole chunk properly, please use something like:

```python
data = b""

expected_size = int.from_bytes(io.recv(8), byteorder="little")

while len(data) < expected_size:
	data += io.recv(expected_size)
```

TCP chunk limit is about 0xfff. Currently the problem with sending bigger chunks properly is not solved in LeForum.

#### How  to read and understand byte diagrams in this (and others) documentation page?
- `{N}` - n-byte long number
- `...` - text
- `[data]` - optional data
- `(OP1/OP2)` - one of presented

Please note that in examples byte structure is split using newlines for readability. In reality, there are none of them. In other words, if you see this in documentation:

```
CNT
{4}   <- counter value
BYE
```

Then the actual request that you should expect/send is:

```
CNT{4}BYE
```

### List of API endpoints:
- [GTHR](#GTHR) - get lethread by ID
- [CTHR](#CTHR) - create lethread
- [FTHR](#FTHR) - find lethread by topic part
- [CMSG](#CMSG) - create lemessage in lethread with provided ID
- [META](#META) - get meta-information about the server
- [LIVE](#LIVE) - alive (used to make sure the connection is still established)

### GTHR
**Description**:
Get LeThread by ID.

**Returns**:
LeThread information and message history. If no LeThread with given ID found, returns `NFND`

**Request syntax**:
```
GTHR
THRID
{8}         <- LeThread ID   
```

**Request actual view**:
```
GTHRTHRID{8}
```

**Successful response syntax**:
```
THRID       <- LeThread ID
{8}        
TPCSZ
{8}         <- Topic size
TPC 
...         <- Topic itself
MSGCNT
{8}         <- Number of messages
MSG         ----------- Message 1 ----------------
{1}         <- Is message posted by LeThread author (aka BY_AUTHOR)
{8}         <- LeMessage ID
{8}         <- LeMessage text size
...         <- LeMessage text
MSGEND      --------- Message 1 End --------------
MSG         ----------- Message 2 ----------------
{1}
{8}
{8}
...
MSGEND      --------- Message 2 End --------------
            And so on
```

**Successful response actual view**:
```
THRID{8}TPCSZ{8}TPC...MSGCNT{8}MSG{1}{8}{8}...MSGENDMSG{1}{8}{8}...MSGEND
```

---

## CTHR
**Description**:
Create LeThread.

**Returns**:
LeThread ID and author token on success. Returns `IDAT` if topic size is not in allowed bounds (see [meta query](#META))

**Request syntax**:
```
CTHR
TPCSZ
{8}         <- Topic size
TPC
...         <- Topic
```

**Request actual view**:
```
CTHRTPCSZ{8}TPC...
```

**Successful response syntax**:
```
THRID
{8}            <- Created LeThread ID
TKN
{TOKEN_SIZE}   <- Author token
```

**Successful response actual view**:
```
THRID{8}TKN{TOKEN_SIZE}
```

---

## FTHR
**Description**:
Find LeThreads by topic.

**Returns**:
IDs of match LeThreads if found at least one. If no LeThread found, returns `NFND`. Return `IDAT` if topic part size is not in allowed bounds (refer to [meta query](#META))

**Request syntax**:
```
FTHR
TPCPSZ
{8}          <- Topic part size
TPCP
...          <- Topic part
```

**Request actual view**:
```
CTHRTPCSZ{8}TPC...
```

**Successful response syntax**:
```
THRID        --------- LeThread 1 -----------
{8}          <- LeThread ID
TPCSZ
{8}          <- Topic size
TPC
...          <- Topic
THRID        --------- LeThread 2 -----------
{8}          <- LeThread ID
TPCSZ
{8}          <- Topic size
TPC
...          <- Topic
             And so on
```

**Successful response actual view**:
```
THRID{8}TPCSZ{8}TPC...THRID{8}TPCSZ{8}TPC...
```

---

## CMSG
**Description**:
Create (post) LeMessage in the LeThread with given ID.

**Returns**:
`OK` on success. Returns `NFND` if no LeThread with given ID was found. Returns `IDAT` if text size if out of bounds (refer to [meta query](#META))

**Request syntax**:
```
CMSG
THRID
{8}               <- LeThread ID to post LeMessage in
TXTSZ
{8}               <- Text size
TXT
...               <- Text
[TKN{TOKEN_SIZE}] <- (optional) Author token
```

**Request actual view**:
```
CMSGTHRID{8}TXTSZ{8}TXT...[TKN{TOKEN_SIZE}]
```

**Successful response syntax**:
```
OK
```

**Successful response actual view**:
```
OK
```

---

## META
**Description**:
Get meta information about server.

**Returns**:
Min&max LeThread topic size, min&max LeMessage text size, number of LeThreads, server version.

**Request syntax**:
```
META
```

**Request actual view**:
```
META
```

**Successful response syntax**:
```
MINTPCSZ
{8}            <- Min LeThread topic size
MAXTPCSZ
{8}            <- Max LeThread topic size
MINMSGSZ
{8}            <- Min LeMessage text size
MAXMSGSZ
{8}            <- Max LeMessage text size
THRN
{8}            <- Number of LeThreads
VERSZ
{8}            <- Size of version string
VER
...            <- Version string
```

**Successful response actual view**:
```
MINTPCSZ{8}MAXTPCSZ{8}MINMSGSZ{8}MAXMSGSZ{8}THRN{8}VERSZ{8}VER...
```

---

## LIVE
**Description**:
Assures the connection between client and server is established.

**Returns**:
`OK` if server is ok

**Request syntax**:
```
LIVE
```

**Request actual view**:
```
LIVE
```

**Successful response syntax**:
```
OK
```

**Successful response actual view**:
```
OK
```
