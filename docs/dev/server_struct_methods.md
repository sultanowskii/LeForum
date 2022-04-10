# LeForum Developer Documentation
## Server inner structure methods
### LeThread
#### `LeThread * lethread_create(char *topic, uint64_t lethread_id)`
**Description**:
Creates new LeThread, if there is no information files with the same id. If there is, then returns LESTATUS_EXST.

**Parameters**: 
`topic` - Topic (aka name) of LeThread. Note that it is copied, therefore you should `free()` it later in your program
`lethread_id` - ID of LeThread

**Return value**:
Pointer to the created LeThread

#### `status_t lethread_delete(LeThread *lethread)`
**Description**:
Safely deletes LeThread. Use this _only_ if you created it using `lethread_create()`.

**Parameters**:
`lethread` - Pointer to LeThread to delete

**Return value**:
LESTATUS_OK on success

#### `uint64_t lethread_message_count(LeThread *lethread)`
**Description**: 
Returns number of LeMessages in lethread.

**Parameters**:
`lethread` - Pointer to LeThread

**Return value**:
Number of LeMessages in the given LeThread

#### `status_t lethread_save(LeThread *lethread)`
**Description**: 
Saves LeThread to the corresponding file.

**Parameters**:
`lethread` - Pointer to LeThread

**Return value**:
LESTATUS_OK on success

#### `status_t lethread_load(LeThread *lethread, uint64_t id)`
**Description**: 
Loads LeThread from the corresponding file.

**Parameters**:
`lethread` - Pointer to LeThread object where information will be loaded to. It should be created by `malloc(sizeof(LeThread))`, don't use `lethread_create()` here
`lethread_id` - ID of the LeThread that is expected to be loaded

**Return value**:
LESTATUS_OK on success. If the corresponding file is not found, LESTATUS_NSFD is returned

#### `status_t lemessages_save(LeThread *lethread)`
**Description**: 
Saves LeMessage history to the corresponding file.

**Parameters**:
`lethread` - Pointer to LeThread message history of which will be saved

**Return value**:
LESTATUS_OK on success

#### `bool_t is_token_valid(LeThread *lethread, const char *token)`
**Description**: 
Comapres author token for the given LeThread and the one to check.

**Parameters**:
`lethread` - Pointer to LeThread, origin token of which will be checked
`token` - Token to check

**Return value**:
TRUE if the token is valid, FALSE if not

---

### LeMessage
#### `LeMessage * lemessage_create(LeThread *lethread, char *text, bool_t by_lethread_author)`
**Description**: 
Creates a new LeMessage and adds it to the given LeThread.

**Parameters**:
`lethread` - Pointer to LeThread where new LeMessage will be posted
`text` - Text of the LeMessage
`by_lethread_author` - Is it posted by LeThread author?

**Return value**:
Pointer to the created LeMessage

#### `status_t lemessage_delete(LeMessage *message)`
**Description**: 
Safely deletes LeMessage. Use this only if you created the LeMessage using lemessage_create().

**Parameters**:
`lemessage` - Pointer to LeMessage to delete

**Return value**:
LESTATUS_OK on success

#### `status_t lemessage_save(LeMessage *lemessage)`
**Description**: 
Saves one LeMessage to the corresponding file (appending to the end of message history file).

**Parameters**:
`lemessage` - Pointer to LeMessage to be saved

**Return value**:
LESTATUS_OK on success

---

### LeAuthor
#### `LeAuthor * leauthor_create(LeThread *lethread, bool_t create_token)`
**Description**: 
Creates a LeAuthor and adds it to the given LeThread.

**Parameters**:
`lethread` - Pointer to LeThread where new LeAuthor will be initialised
`create_token` - Should token be generated for the LeAuthor?

**Return value**:
Pointer to the created LeAuthor

#### `status_t leauthor_delete(LeAuthor *author)`
**Description**: 
Safely deletes LeAuthor. Use this only if you created the LeMessage using `leauthor_create()`.

**Parameters**:
`message` - Pointer to LeAuthor to delete

**Return value**:
LESTATUS_OK on success

#### `status_t leauthor_load(LeThread *lethread)`
**Description**: 
Loads the author of the lethread from the corresponding file.

**Parameters**:
`lethread` - Pointer to LeThread, information about author of which will be loaded (lethread->author has to be a valid pointer to LeAuthor object on the heap)

**Return value**:
LESTATUS_OK on success


#### `status_t leauthor_save(LeThread *lethread)`
**Description**: 
Saves author of the lethread to the corresponding file.

**Parameters**:
`lethread` - Pointer to LeThread, author of which will be saved

**Return value**:
LESTATUS_OK on success


---

### LeCommand
No methods, this struct is only used for data transfer.

---

### LeCommandResult
No methods, this struct is only used for data transfer.

---

### LeClientInfo
No methods, this struct is only used for data transfer.