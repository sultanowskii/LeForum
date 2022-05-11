# LeForum Developer Documentation
## Server threads
#### Main Thread
**Description**: Starts and finishes server job, runs all others.

#### LeThread/LeMessage/LeMessages/LeAuthor Query Manager Threads
**Description**: Securely (preventing data race) change data files.

#### Client Handler Threads
**Description**: Handle clients and communicate with them.
