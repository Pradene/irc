# IRC

This project implements an IRC (Internet Relay Chat) server capable of handling various commands, including the /kick command, which allows operators to remove users from channels.

## Features
- Command Handling: Supports standard IRC commands such as /join, /part, /msg, and /kick.
- Channel Management: Allows users to join and leave channels, with appropriate notifications.
- User Management: Operators can manage users within channels, including the ability to kick users.

## Requirements
- C++ Compiler: Ensure you have a C++ compiler that supports C++98.
- Make: Used for building the project.
- Libraries: No external libraries are required; the project uses standard C++ libraries.

## Installation

1. Clone the repo:
    ```bash
    git clone https://github.com/Pradene/irc.git
    ```
2. Build the project:
     ```bash
     make
     ```

## Usage

To run the IRC server:

```bash
./ircserv [port] [password]
```
