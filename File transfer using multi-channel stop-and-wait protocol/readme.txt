Readme for Q1

Commands for compiling code

$ gcc server.c -o server
$ gcc client.c -o client
$ ./server
(Below command in a new terminal)
$ ./client

Implementation: 
    I handled the problem of mlyiple timers using select() system call where, If first channel drops
a packet then, the timer will wait till the second also drops one then both the packets are retransmitted
and the system contnues. With use of select() , FD_ISSET() and other functions I had checked which TCP socket
has received a packet and acted accordingly. For Q1, I handled the case where filesize is not a multiple
of PACKET_SIZE. If the server wants to drop the packet receievd through channel 0, the ack for it won't
be sent and the client doesn't take any action untill timeout occurs. For Q1, I implemented Dropping of packets by
dropping one packet in 100/PDR packets. Eventhough, this is not random drop, this method gives
statistically better results.