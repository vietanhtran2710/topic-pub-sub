# topic-pub-sub
Topic Publisher and Subscriber for Network Programming project
# Protocol
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
