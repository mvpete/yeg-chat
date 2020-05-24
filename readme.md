yeg-cpp-chat specification                          
===
May 2020

Author: Peter Lorimer


Abstract

    This protocol was designed to illustrate network communications for the YEG CPP Meetup.
    The point is to be straightforward and simple, to illustrate some of the challenges
    with network programming and asynchronous IO. It is by no means free of error. It is a
    simple binary protocol defined as follows.

__1. Overview__

    It's simple, the yeg-chat is a simple centralized chat service, whereby there is a single 
    server serving N clients. When a client connects, they handshake with the server, and they
    are then able to send messages to the group, or to direct message (dm) users individually.

    Messages are in the format of:
```
    0               1               2               3               4 
    0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0
    +---------------------------------------------------------------+
    |                          Signature                            |
    +------------------------------+--------------------------------+
    |       Function Code          |             Length             |
    +------------------------------+--------------------------------+
    |                                                               |
    |                           Message                             |
    |                                                               |
    +---------------------------------------------------------------+
```


    Each packet is identified with a signature, followed directly by a 2-byte Function Code, 
    which defines a variable length message to follow.

    **Note:** All multi-byte numbers are network host ordering.

    string := <unsigned short len> [ ascii characters ]

__2. Function Codes__

```
    0x0001 - Hello World! 
    0x0002 - Broadcast message
    0x0003 - Receive broadcast

    0xFFFF - Error
```

__3. Interaction__

    The client initiates the connection with the server, and sends a message
    containing exactly the length of the username, and the ASCII username
    in the message

a. Beginning Handshake

    [ Client ]                                             [ Server ]
         |
         |------------[YEGC][0x0001][0x0006][mvpete]---------->|
         |                                                     |
         |<--------------[YEGC][0x0001][0x0000]----------------|

Client begins by sending a [YEGC] packet, with the length of username followed by the ASCII username.

b. Sending a message


    [ Client A ]                                    [ Server ]                                 [ Client B ]
         |
         |----[YEGC][0x0002][ msg length ][message]---->|                                            |
         |                                              |                                            |
         |                                              |----[YEGC][0x0003][   total length    ]     |
         |                                              |    [ ul ][ username ][ ml ][ message ]---->|

A user sends a message by sending a 0x0002 FC, followed by the length of the message and the message (ASCII). 
The message is then broadcast to all connected users, with the 0x0003 FC, the packet will contain a 2 byte
total length. Following the length, a string username, and a string message.

__4. Building__

On Windows, build the solution using /std:c++17 with Visual Studio 2019 (Version 16.4.2).

__5. To Run__

./yeg-chat-server

./yeg-chat-client [ hostaddr ] [ username ]