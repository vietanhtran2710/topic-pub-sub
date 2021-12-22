# topic-pub-sub
Topic Publisher and Subscriber for Network Programming project
# Build and Run
All 3 programs are built with Qt Creator 4.11.0 - Qt 5.12.8
# Architecture
![alt text](https://github.com/vietanhtran2710/topic-pub-sub/blob/main/architecture.jpg?raw=true)
All threads with signal-slot connections are thread-safe with all threads have equal priorities.
# Protocol
![alt text](https://github.com/vietanhtran2710/topic-pub-sub/blob/main/protocol.jpg?raw=true)
## PUBLISHER
```
{
  "command": "STOP PUBLISHING",
  "topic": [topic]
}
{
  "command": "PUBLISHING",
  [data]
}
{
  "command": "START PUBLISHING",
  "topic": [topic]
}
{
  "command": "QUIT"
}
```
## SUBSCRIBER
```
{
  "command": "CONNECT",
{
Server then respond with CONNECTACK
{
  "command": "STOP SUBSCRIBING",
  "topic": [topic]
}
{
  "command": "GET ALL TOPICS",
}
{
  "command": "START SUBSCRIBING",
  "topic": [topic]
}
{
  "command": "QUIT"
}

```
## SERVER
```
{
  "command": "CONNECTACK",
{
Server transfer data from publisher to subscriber
{
  [data],
  "topic": [topic]
}
```
