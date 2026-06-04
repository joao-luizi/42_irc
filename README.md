*This project has been created as part of the 42 curriculum by joaomigu, ddiogo-f.*

# ft_irc

## Description

ft_irc is an Internet Relay Chat server written in C++98, built from scratch as part of the 42 common core curriculum. The server speaks the IRC protocol over TCP/IP and lets real IRC clients connect, register, join channels, and exchange messages — both public broadcasts and private messages — exactly as they would against any production IRC network.

The implementation follows RFC 1459 as its base specification, with parts of RFC 2812 and the [Modern IRC](https://modern.ircdocs.horse/) documentation used where the original RFC is ambiguous or outdated. It is a single-process, single-`poll()` server: all socket I/O (accept, read, write) is non-blocking and multiplexed through one event loop, with no threads and no forking. The reference client used during development is **HexChat**, with `nc -C` used for raw protocol inspection.

The project does **not** implement an IRC client and does **not** implement server-to-server communication — connections are strictly client-to-server.

## Features

### Registration commands
- `PASS` — submit the connection password
- `NICK` — set or change the nickname
- `USER` — set username and real name
- `QUIT` — disconnect with an optional reason

### Channel commands
- `JOIN` — join (or create) a channel, with optional key
- `PART` — leave a channel
- `PRIVMSG` — send a message to a user or a channel
- `TOPIC` — view or set the channel topic
- `WHO` — list members of a channel

### Operator commands
- `KICK` — eject a user from a channel
- `INVITE` — invite a user to a channel
- `MODE` — change channel modes:
  - `+i` / `-i` — invite-only
  - `+t` / `-t` — restrict `TOPIC` to operators
  - `+k <key>` / `-k` — set or remove the channel key
  - `+o <nick>` / `-o <nick>` — grant or revoke operator privileges
  - `+l <limit>` / `-l` — set or remove the user limit

## Instructions

### Build

```sh
make           # compile (release build)
make DEBUG=1   # compile with debug logging and the SHOW command enabled
make clean     # remove object files
make fclean    # remove object files and the binary
make re        # fclean + make
```

The release build compiles with `-Wall -Wextra -Werror -std=c++98`. The debug build defines the `DEBUG` macro, enabling `DEBUG_MSG(...)` traces and the in-protocol `SHOW server|client|channel <name>` command for live state inspection.

### Run

```sh
./ircserv <port> <password>
```

- `port` — listening port, between `1024` and `65535`
- `password` — connection password, non-empty and without spaces

Example:

```sh
./ircserv 6667 hunter2
```

### Connecting from another machine on the same LAN

To connect from a different machine (useful when running the server in WSL and connecting from a peer on the school network), the client needs the server host's IP address.

On the server machine, find the gateway / LAN IP:

```sh
ip route | grep default | awk '{print $3}'
```

Then in HexChat:

1. **Network List** → **Add** → name the entry (e.g. `ft_irc`)
2. **Edit…** → set the server line to `<ip>/<port>` (e.g. `10.11.249.42/6667`)
3. Set the **Password** field to the password passed to `./ircserv`
4. Set the **Nick name** and **User name**
5. **Connect**

For raw protocol testing, `nc -C` is the recommended tool — it sends CRLF line endings, matching the IRC wire format:

```sh
nc -C 127.0.0.1 6667
```

## Resources

### Technical references

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) — foundational guide to sockets, `poll()`, non-blocking I/O, byte order, and the client-server model
- [RFC 1459 — Internet Relay Chat Protocol](https://datatracker.ietf.org/doc/html/rfc1459) — the original 1993 IRC specification; the base reference for this project
- [RFC 2812 — IRC Client Protocol](https://datatracker.ietf.org/doc/html/rfc2812) — the 2000 update focused on client-side protocol behaviour
- [Modern IRC documentation](https://modern.ircdocs.horse/) — community-maintained reference describing IRC as it is actually implemented today, useful when the RFCs are ambiguous
- [HexChat documentation](https://hexchat.readthedocs.io/) and [HexChat homepage](https://hexchat.github.io/) — reference client used for testing

### Language and system references

- [cppreference.com](https://en.cppreference.com/Main_Page) — C++ standard library and language reference
- [cprogramming.com](https://www.cprogramming.com/) — C and C++ tutorials and reference material
- [Linux man pages (man7.org)](https://man7.org/linux/man-pages/index.html) — authoritative reference for `socket`, `poll`, `recv`, `send`, `fcntl`, and other system calls used by the server
- [IBM `fcntl()` documentation](https://www.ibm.com/docs/en/zvm/7.4.0?topic=functions-fcntl) — detailed reference for the `fcntl()` system call, used to set sockets to non-blocking mode

### Video tutorials

- [Sockets in C — TCP Server & Client](https://www.youtube.com/watch?v=0EiQwUiHzCw)
- [IRC Protocol walkthrough](https://www.youtube.com/watch?v=bA1VMjShQUk)
- [Building an IRC server in C++](https://www.youtube.com/watch?v=Mnm_40SmLXQ)
- [hton & Big-Endian/Little-Endian Problem](https://www.youtube.com/watch?v=hcUku_6G5Cg)

### Reference implementations from other 42 students

- [annafenzl/42-ft_irc](https://github.com/annafenzl/42-ft_irc) — a 42Heilbronn implementation focused on multi-client handling with non-blocking I/O
- [ThePhoenix77/IRC-server](https://github.com/ThePhoenix77/IRC-server) — a C++98 implementation following RFC 2812, with channel and operator support

### AI usage

AI tools (Claude by Anthropic, NotebookLM by Google) were used during development for the following tasks:

- **Research** — exploring the IRC protocol, the meaning of numeric replies, common implementation patterns, and the differences between RFC 1459 and RFC 2812
- **Debugging** — analysing unexpected client behaviour, tracing protocol-level bugs (e.g. malformed numeric replies, broken `WHO` parsing), and reasoning about edge cases in the `poll()` loop
- **Test scenario generation** — producing sequences of raw IRC commands for `nc -C` and HexChat to exercise registration, channel modes, and operator privileges
- **Documentation** — producing internal reference documents in Portuguese for cross-session continuity, and drafting this README

All code in this repository was written and reviewed by the authors. AI was used as an assistive tool — for investigation, review, and documentation — and never as a code generator for parts of the project the authors did not fully understand. Every line of the submitted server is one the authors can explain and defend.
