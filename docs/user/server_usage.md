# LeForum User Documentation
## Server usage
#### General
LeForum Server has no interface. After parsing all the command line arguments, server runs and falls into an infinite loop of accepting clients. Therefore, as mentioned above, you should run it in background (the best option is Docker with volume to save all the data).

---

#### Command line arguments
Short exaplanation of all the arguments is available by running program with `--help` flag.

List of arguments (aka flags):
    - `-c, -max-connections=NUM`- Max number of active connected clients. This argument is passed to `listen(2)`, therefore an actual behaviour may differ from platform to platform. Don't rely on it too much. The default value is **100**
    - `-h, --host=ADDR` - Host IPv4 address to bind server on. The default is **0.0.0.0**
    - `-m, --hello-message=TEXT` - Hello message on startup. Default value is **Have a great day!**
    - `-p, --port=NUM` - Port to bind server on. Default value is **7431**
    - `-t, --timeout=NUM` - Timeout for client respond (in seconds). Default value is **3**.
    - `-?, --help` - Help message.
    - `--usage` - Short usage message.
    - `-V, --version` - Print program version.
