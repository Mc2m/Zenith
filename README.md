Zenith
======

A Multi-State manager for Lua

Zenith was conceived with the goal of managing lua states and communication from C. The main issue with current state managers such as lua lanes and linda is that it requires a boot script to initialize the states from lua, which can requires more communication from C. This model allows developers to directly use C to generate and manage the states.

Communication is currently implemented using pipes, although there are plans to use shared tables.
Pipe communication support four method of access :
	- Send : send the data and returns
	- Wait : send the data and wait for an answer until the timeout expire
	- Listen : wait for data until timeout expire
	- Receive : wait for data
	
Pipes also save the data groups so the receiver can get one group at a time. For example, if the sender do pipe:send(1,2,3) then pipe:send(4,5,6) and some time later the receiver execute pipe:receive(0), the output will be {1,2,3}. On the other hand, if pipe:receive() is executed, the output will be {1,2,3,4,5,6}.

To allow more efficient communication, Zenith also implements a request manager system and an execute request as example. The manager simply has to be created with the desired state as input. This manager currently requires to be run manually (threaded implementation is planned). It's purpose is to identify the request and execute it. It will check on each pipe for requests. Request templates must be added manually to the request manager. There is currently no way to prevent certain threads to use specific requests.

A word of warning : this is still a very early version and although the system work, documentation is very poor. One of the things that need to be changed is the access to the pipes from lua. Right now, to access a pipe, the user has to use Zenith.Pipe.pipes.<name_of_the_pipe>. This method is not clear enough and will be modified. Lua script containing access to such functions seems like a better way to go.
