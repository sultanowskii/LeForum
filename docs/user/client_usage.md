# LeForum User Documentation
## Client Usage
#### General
Currently the client is a CLI (Command Line Interface) program. To navigate, you should look at the appearing menu, for example:

```
Available commands:
1 - Server
2 - Thread
3 - Settings
4 - Exit

[*] >
```

And choose where you want to go. For instance, I want to go to Server section, ok, number 1, I type it...

```
Available commands:
1 - Server
2 - Thread
3 - Settings
4 - Exit

[*] > 1
```

... and hit Enter:

```
Available commands:
1 - Server
2 - Thread
3 - Settings
4 - Exit

[*] > 1

Available server commands:
1 - Connect
2 - Server information
3 - Server history
4 - Back

[Server] >
```

As you can see, we moved to Server section, just as we wanted to. To go back, just find an option "Back" in the menu.

Another thing to keep in mind is not to press Ctrl+C or Ctrl+D, because it causes some messes in stdin.

---

#### Sections
There are several sections of the program, here is the list of them:

- **Main section**:
    - **Server**: Move to Server section
    - **Client**: Move to Client section
    - **Thread**: Move to Thread section
    - **Exit**: Exit the program
- **Server section**:
    - **Connect/Disconnect**: Well, connect/disconnect to some LeForum server
    - **Server information**: Information about active server
    - **Server history**: History of connected servers
    - **Back**: Go back
- **Thread section**:
    - **Create thread**: Create thread in an active server
    - **Find thread**: Find thread in the active server, and then make one of them active
    - **Thread info**: Get meta-information about thread
    - **Message history**: Get message history of active thread
    - **Post message**: Send (aka post) message in the active thread
    - **Back**: Go back
- **Settings section**:
    - **Back**: Go back
