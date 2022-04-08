# LeForum Developer Documentation
## List of server threads:
- Main Thread - starts and finishes server job, runs all others.
- LeThread/LeMessage/LeMessages/LeAuthor Query Manager Threads - threads that securely (preventing data race) change data files.
- Client Handler Threads - threads that handle clients and communicate with them.