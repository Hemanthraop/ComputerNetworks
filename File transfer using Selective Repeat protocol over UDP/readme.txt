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


Readme for Q2

Commands for compiling code

$ gcc server.c -o server
$ gcc relay1.c -o relay1
$ gcc relay2.c -o relay2
$ gcc client.c -o client
$ ./server
(Below command in a new terminal)
$ ./relay1
(Below command in a new terminal)
$ ./relay2
(Below command in a new terminal)
$ ./client

Implementation:
    Here I implemented droping by generating a random number%100 and by checking if it is less than
PDR ,then drop it, otherwise don't. I tackled out-of-order delivery by using a buffer(char array) of length 
PACKET_SIZE*4 here,4 is my window size(a mcaro). If a packet at the start of the receiver window is received
by the receiver, the therequired amount of buffer is written to the file and buffer is shifted by required
number of places. I implemented using 2 files for relays as the instruction of using 1 file for relay was so 
lately known to me.

Assumption:
    Filesize is multiple of PACKET_SIZE. If it is not so my program will write some extra '\0's at the end of the file.
