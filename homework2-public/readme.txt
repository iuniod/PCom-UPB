*-----------------------------------------------------------------------------*
* HOMEWORK 2 PCOM							   Iustina-Andreea Caramida 322CA *
*-----------------------------------------------------------------------------*
* To be sure, I used 2 sleep days for this homework.						  *
*-----------------------------------------------------------------------------*
* Structure of the archive:						                              *
* - defines.h - contains all the defines used in the program + headers        *
* - helper.h - contains the structures used in the program				  	  *
* - helper.c - contains the send and receive functions					  	  *
* - tcp_client.c - contains the implementation of the TCP client			  *
* - server.h - contains the structure of the server + the functions used	  *
* - server.c - contains the implementation of the server					  *
*-----------------------------------------------------------------------------*
* How to run the program:													  *
* - make all - compiles the program										  	  *
* - make clean - deletes the object files and the executable				  *
* - ./server - runs the server [port]										  *
* - ./subscriber - runs the subscriber [id_client] [ip_server] [port_server]  *
*-----------------------------------------------------------------------------*
* What I implemented:														  *
* - I implemented all the functionalities required by the homework.			  *
* - I used C++ because I am more familiar with it and it conains a lot of	  *
*   useful data strucures.			  										  *
* - I used unordered_map to store the clients and their data because it is    *
*   faster than map and I can have a map between the client and its data.	  *
* - I used epoll to handle the multiple connections because I am familiar with*
*   it from SO.																  *
* - I disabeled the Nagle algorithm as it was specified in the homework, 	  *
*   although when I send a message, I send firstly the length of the message  *
*   and then the message itself. And when I receive a message, I receive 	  *
*   the length of the message and then the message. I did this because I 	  *
*   wanted to be sure that I receive the correct message.					  *
* - I parse the notification in TCP client because I find it easiser and 	  *
*	faster to send the message to the client and let the client parse it. 	  *
* - I used a queue to store the messages for the clients that are disconnected*
*-----------------------------------------------------------------------------*
* Implementation of the server												  *
* - The server is implemented using an unordered_map to store the clients and *
*   their data such as the topics they subscribed to, the queue of messages,  *
*   if they are connected or not and the socket they are connected to.		  *
* - Each client contains its id, the socket it is connected to, the port and  *
*   the ip address.														  	  *
* - It uses 2 sockets to communicate with the clients: one for TCP and one for*
*   UDP.																	  *
* - The server handles the following cases:								  	  *
*		- one TCP client tries to connect to the server						  *
*		- it receives a message from a UDP client							  *
*		- it receives a message from a TCP client							  *
*		- it receives a command from the keyboard							  *
*																			  *
* 1. TCP connection															  * 
* A new TCP client tries to connect to the server. The server checks if there * 
* is already a client with the same id. If there is, it checks if that client *
* is disconnected or not. Tus, if the client is disconnected or there is no   *
* client with the same id, the server adds the new client to the map and      *
* prints the message "New client [id_client] connected from [ip_client]:      *
* [port_client].". Otherwise, it prints the message "Client [id_client] 	  *
* already connected.". 														  *
* 																			  *
* 2. UDP message															  *
* The server receives a message from a UDP client. That message contains a new*
* notification for a specific topic. The server sends the message to all the  *
* TCP clients that are subscribed to that topic and are connected. If there is*
* a client that is subscribed but is disconnected, the message is added to the*
* queue of messages for that client, and it will be sent when the client 	  *
* connects again.															  *
* 																		      *
* 3. TCP message															  *
* The server receives a message from a TCP client. That message could contain:*
*		- exit [id] - the client with the id [id] wants to disconnect from the*
*					  server. The server checks if there is a client with that*
*					  id. If there is, it disconnects the client.			  *
*		- subscribe [id] [topic] [SF] - the client with the id [id] wants to  *
*										 subscribe to the topic [topic] with  *
*										 the SF [SF]. The server checks if	  *
*										 there is a client with that id. If	  *
*										 there is, it adds the topic to the	  *
*										 list of topics the client is	  	  *
*										 subscribed to, along with the SF.	  *
*		- unsubscribe [id] [topic] - the client with the id [id] wants to	  *
*									 unsubscribe from the topic [topic]. The  *
*									 server checks if there is a client with  *
*									 that id. If there is, it removes the	  *
*									 topic from the list of topics the client *
*									 is subscribed to.						  *
*																			  *
* 4. Keyboard commands														  *
* The server receives a command from the keyboard. The only command it can	  *
* receive is "exit". If it receives this command, it disconnects all the	  *
* clients and exits.														  *
*-----------------------------------------------------------------------------*
* Implementation of the TCP client											  *
* - The client creates a TCP socket and tries to connect to the server.		  *
* - If the connection is successful, it sends its id to the server.			  *
* - The client handles the following cases:								  	  *
*		- it receives a message from the server								  *
*		- it receives a command from the keyboard							  *
*																			  *
* 1. Server message															  *
* The client receives a message from the server. That message could contain   *
* only one notification for a specific topic. The clients uses the function   *
* "parse_message" to parse the message and print it according to the format.  *
* 																			  *
* 2. Keyboard commands														  *
* The client receives a command from the keyboard. The commands it can receive*
* are:																		  *
*		- exit - the client wants to disconnect from the server. It sends the *
*				 message "exit [id]" to the server and disconnects.			  *
*		- subscribe [topic] [SF] - the client wants to subscribe to the topic *
*								   [topic] with the SF [SF]. It sends the 	  *
*								   message "subscribe [id] [topic] [SF]" to   *
*								   the server.								  *
*		- unsubscribe [topic] - the client wants to unsubscribe from the topic*
*								 [topic]. It sends the message "unsubscribe   *
*								 [id] [topic]" to the server.				  *
*-----------------------------------------------------------------------------*
