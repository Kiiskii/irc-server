# irc-server

A C++ implementation of a basic IRC (Internet Relay Chat) server, built according to the IRC protocol specifications.  
This project focuses on low-level networking, event-driven I/O, protocol parsing, and managing multiple clients concurrently.

The server is designed to be used with real IRC clients (such as `irssi`) and handles multiple connections using `epoll`.

This project was developed in collaboration with
[Trang Pham](https://github.com/TrangPham93) and
[Karoliina Hiidenheimo](https://github.com/kaloliina).

---

## Project Overview

**irc-server** is part of the 42 / Hive Helsinki curriculum.  
The goal of the project is to implement a functional IRC server from scratch, without relying on existing IRC libraries.

---

## Features

- TCP server using IPv4
- Non-blocking sockets with `epoll`
- Multiple simultaneous clients
- User authentication (`PASS`, `NICK`, `USER`)
- Channel creation and management
- Joining and leaving channels
- Private messages and channel messages
- Operator privileges
- Graceful client disconnection handling
- Compatible with real IRC clients (tested with `irssi`)

---

## Usage

### 1. Clone the repository

```bash
git clone https://github.com/Kiiskii/irc-server.git
cd irc-server
```

### 2. Build the project

```bash
make
```
This will produce the ircserv executable.

### Running the Server

```bash
./ircserv <port> <password>
```
Example:
```bash
./ircserv 6667 mypassword
```
Then on a new terminal instance you can connect to the server with irssi.

## Testing

The server has been tested with:

- `irssi`
- Multiple simultaneous clients
- Invalid and malformed commands
- Sudden client disconnects
