# le-forum
LeForum is an anonymous forum (server&client) working via TCP.

## Features
- Written in C
- Uses common C UNIX libraries
- Allows to view thread history, write messages and create your own threads
- LeForum Server API allows to easily create your own client

## Installation
First, download repository and enter **src/** directory:
```bash
git clone https://github.com/sultanowskii/le-forum
cd le-forum/src
```

Then compile server/client using following commands:
```bash
make c-server
make c-client
```

Compiled binary/-ies will appear in the **le-forum/** directory.

## Contribute
If you are interested in this project and want to implement a new feature or to fix some bug - feel free to open pull request.

## Support
First of all, read "--help" manual. For further explanation, go to [docs](docs/README.md).

This project is not fully supported (and will get less and less of it as times passes). It's more about getting C experience for me.

## License
This project is licensed under GPL-3.0 License.
