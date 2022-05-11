# LeForum Documentation
LeForum is an anonymous forum (server&client) working via TCP written in C.

Usage is presented in [User docs](user/README.md)

This file contains general information about the project. For deeper explanations, please refere to [Developer docs](dev/README.md).

Known issues are [here](issues.md).

## Things to keep in mind using LeForum:
- This is not a full-time project of my life - it's more about practice and writing code, don't get it too serious.
- It's developed under Linux platform, it uses several UNIX libraries (like unistd.h, argp.h, etc), therefore it could only be used on Linux.
- I don't have plans of porting neither client nor server for Windows.
- Client uses **~/.leforum_client** directory, server uses **.leforum_server**. If you want to run server binary somewhere else from the current location, don't forget to move the directory contents too.