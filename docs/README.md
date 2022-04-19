# LeForum Documentation
LeForum is an anonymous forum (server&client) working via TCP written in C.

This file contains general information about the project. For deeper explanations, please refere to [Developer docs](dev/README.md).

Known issues are [here](issues.md).

## Things to keep in mind using LeForum:
- I screwed up a whole ocean of things, it contains bugs. Don't take this project as a good example, it's quite messy.
- This is not a full-time project of my life - it's more about practice and writing code, don't get it too serious.
- It's developed under Linux platform, it uses several UNIX libraries (like unistd.h, argp.h, etc), therefore it could only be used on Linux.
- I don't have plans of porting neither client nor server for Windows.
- Both client and server use **.data/** directory near them. If you want to move binary somewhere else from the current location, don't forget to move the folder too.
- Once stable working version of both server and client is released, I will not support this project due to lack of time and an amount of mess I did here.