# Specstutis
- a peer-to-peer LAN messaging program written in C 

## How it works
### Peer Discovery
- when the program starts, a client will start sending broadcast UDP packets on the LAN in 5 second intervals containing the following info
```c
typedef struct {
    uint8_t version;
    uint8_t message_type; // discover=0, or disconnect=1
    uint16_t listening_port;
    char[32] username;
} peer_discovery_packet;

```
- users can disconnect gracefully by sending a peer discovery packet with the message_type of 1, or if they dont send a peer discovery packet for 20 seconds they will be timed out

### Chat Messages
- then you can select a peer from the active peers to chat with 
- a TCP connection is setup for chatting and packets are sent in the following format 
```c
typedef struct {
    uint8_t version;
    uint8_t message_type; // chat=0, close_connection=1
    char[32] from_usr;
    char[1024] message_data;
} chat_packet;

```
- when the program starts, `listen_chat_conns()` is called which is a pthread that will run the whole program listening and accepting incoming TCP connections.
- when a chat packet is received on the thread, it first checks that its a valid packet then it will add it to the global incoming messages buffer that will be processed by the TUI
- the TUI has an array of size `MAX_PEERS`(64) for chat history. each element is a circular array of length `MAX_HISTORY_LEN` (8192) that stores the recent messages.
- when a chat is being sent ...