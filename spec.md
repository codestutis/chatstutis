# Specstutis
- a peer-to-peer LAN messaging program written in C 

## How it works
### Peer Discovery
- when the program starts, the host will start sending broadcast UDP packets on the LAN in 5 second intervals containing the following info
```c
typedef struct {
    uint8_t version;
    uint8_t message_type; // discover=0, or disconnect=1
    uint16_t listening_port;
    char[32] username;
} peer_discovery_packet;

```

- then you can select a peer from the active peers to chat with 
- a TCP connection is setup for chatting and packets are sent in the following format 
```c
typedef struct {
    uint8_t version;
    uint8_t message_type; // chat=0, close_connection=1
    char[1024] message_data;
} chat_packet;

```

