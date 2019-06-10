Second generation rewrite of libixp from scratch. Started with libjyq as a C++
rewrite of libixp. This library is written from scratch in C++17.

The library is broken up into several layers

1) Connection
2) Message
3) Logic

The connection layer handles the interface between the outside world and this library. 
It turns the connection into an input and output stream over a raw "byte stream" 
(at least conceptually). This layer is responsible for computing the length of a
message. Concepts like dialing a server and responding to a client are handled at this
level.

The message layer interfaces with the connection layer and provides a consistent
view in the form of 9p2000 messages. It is responsible for reading and writing
messages. This layer also provides the structures that the logic layer operates on. 
The raw messages are converted into more palatable data structures. 

The logic layer is the layer which operates on the messages provided by the 
message layer. A server listens for requests and responds with reciepts. A client
sends requests to a server and listens for reciepts. 
