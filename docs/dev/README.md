# LeForum Developer Documentation
## Server
LeForum Server is based on an infinite loop of accepting clients and handling them in separate threads.

#### List of threads:
- Main Thread - starts all others.
- LeThread/LeMessage/LeMessages/LeAuthor Query Manager Threads - threads that securely (preventing data race) change data files.
- Client Handler Threads - threads that handle client and communicate with them.

#### API endpoints (full reference: [endpoints](endpoints.md)):
- GTHR - get lethread by ID
- CTHR - create lethread
- FTHR - find lethread by topic part
- CMSG - create lemessage in lethread with provided ID
- META - get meta-information about the server
- LIVE - alive (used to make sure the connection is still established)

#### Inner data structures (the full list is [here](structs_server.md)):
