#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdbool.h>

#define PACKET_SIZE 100
#define PDR 10             		//in Percentage
#define RETRANSMISSION_TIME 2		//in seconds
#define PORT1 12344
#define PORT2 12345

struct packet{
    int payload_size;
    int seq;
    bool is_last;       //This is TRUE(1) if the packet is the last packet.
    bool is_data;       //This is TRUE(1) if the packet is DATA packet, FALSE(0) if ACK packet.
    bool channel_id;    //Channel ID : 0(FALSE), 1(TRUE).
    char payload[PACKET_SIZE + 1];
};
