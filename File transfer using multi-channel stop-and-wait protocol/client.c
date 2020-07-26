#include "packet.h"

bool is_closed[2];
bool all_sent;
/*
struct packet{
    int payload_size;
    int seq;
    bool is_last;       //This is TRUE(1) if thye packet is the last packet.
    bool is_data;       //This is TRUE(1) if the packet is DATA packet, FALSE(0) if ACK packet.
    bool channel_id;    //Channel ID : 0(FALSE), 1(TRUE).
    char payload[PACKET_SIZE];
};
*/
int main(){

    //Dividing file and preparing packet strucures

    FILE* fp=fopen("input.txt","r");
    
    //fseek(fp,0,2);
    fseek(fp, 0L, SEEK_END);
    int file_size=ftell(fp);
    //printf("File size: %d\n",file_size);
    fclose(fp);
    
    int total_no_of_pkts;
    if(file_size%PACKET_SIZE==0)
        total_no_of_pkts=file_size/PACKET_SIZE;
    else
        total_no_of_pkts=(file_size/PACKET_SIZE)+1;
    
    struct packet pkts[total_no_of_pkts];
    
    fp=fopen("input.txt","r");
    int offset=PACKET_SIZE,curr=-1;
    
    for(int i=0;i<file_size;i++){
        if(offset==PACKET_SIZE){
            offset=0;
            curr++;
            pkts[curr].seq=curr;
            pkts[curr].payload_size=0;
            if(curr==total_no_of_pkts-1)
                pkts[curr].is_last=true;
            else
                pkts[curr].is_last=false;
            pkts[curr].is_data=true;
            
        }
        
        //fscanf(fp,"%c",&ch);
        fscanf(fp,"%c",&((pkts[curr].payload)[offset]));
        pkts[curr].payload_size++;
        //printf("%c at position %d in msg %d\n",((pkts[curr].payload)[offset]),offset,curr);
        offset++;
        
    }
    fclose(fp);

    //Socket Creation and connecting



    int sock_fd[2];
    int c[2];
    struct sockaddr_in serverAddr[2];
    for(int i=0;i<2;i++){
        sock_fd[i]=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        //printf("Socket %d created\n",sock_fd[i]);

        memset(&(serverAddr[i]),0,sizeof(serverAddr[i]));

        serverAddr[i].sin_family=AF_INET;
        serverAddr[i].sin_port=htons((i==0)?PORT1:PORT2);
        serverAddr[i].sin_addr.s_addr=inet_addr("127.0.0.1");

        c[i]=connect(sock_fd[i],(struct sockaddr*) &(serverAddr[i]),sizeof(serverAddr[i]));
        //delay(100);
        
        for (int c = 1; c <= 3276; c++)
            for (int d = 1; d <= 32767; d++)
                {}
    }
    /*
    char msg[5];
    strcpy(msg,"Hi");
    int bytesSent;
    struct packet pkt[4];
    memset(&(pkt[0]),0,sizeof(struct packet)); 
    bytesSent = send (sock_fd[0], &(pkt[0]), sizeof(struct packet), MSG_NOSIGNAL);
    memset(&(pkt[1]),0,sizeof(struct packet)); 
    pkt[1].channel_id=true;
    pkt[1].seq++;
    bytesSent = send (sock_fd[1], &(pkt[1]), sizeof(struct packet), MSG_NOSIGNAL);
    memset(&(pkt[2]),0,sizeof(struct packet)); 
    pkt[2].channel_id=false;
    pkt[2].seq+=2;
    bytesSent = send (sock_fd[0], &(pkt[2]), sizeof(struct packet), MSG_NOSIGNAL);
    memset(&(pkt[3]),0,sizeof(struct packet)); 
    pkt[3].seq+=3;
    pkt[3].is_last=true;
    bytesSent = send (sock_fd[0], &(pkt[3]), sizeof(struct packet), MSG_NOSIGNAL);
    */

	fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sock_fd[0],&readfds);
    FD_SET(sock_fd[1],&readfds);

	struct timeval timeout;
	timeout.tv_sec=RETRANSMISSION_TIME;  
	timeout.tv_usec=0;

    int activity;// = select( ((sock_fd[0]<sock_fd[1])?sock_fd[1]:sock_fd[0]) + 1 , &readfds , NULL , NULL , NULL);
    int valread;
    struct packet recvd_pkt;
    int maxsd;
    //for(int j=0;j<4;j++)
	int unacked[2]={-1};
	
	unacked[0]=0;
	pkts[0].channel_id=0;
	send (sock_fd[0], &(pkts[0]), sizeof(struct packet), MSG_NOSIGNAL);
    printf("SENT PKT: Seq. No 0 of size %d Bytes from channel 0\n",pkts[0].payload_size);
	all_sent=pkts[0].is_last;
	if(!all_sent){
		unacked[1]=1;
		pkts[1].channel_id=1;
		send (sock_fd[1], &(pkts[1]), sizeof(struct packet), MSG_NOSIGNAL);
		printf("SENT PKT: Seq. No 1 of size %d Bytes from channel 1\n",pkts[1].payload_size);
        all_sent=pkts[1].is_last;
	}
	else{
		unacked[1]=-1;
	}
    while(!(unacked[0]==-1&&unacked[1]==-1)){

        FD_ZERO(&readfds);
        
        if(is_closed[0]&&is_closed[1]){
            break;
        }
        maxsd=0;
        if(!is_closed[0]){
            FD_SET(sock_fd[0],&readfds);
            maxsd=((sock_fd[0]>maxsd)?sock_fd[0]:maxsd);
        }
        if(!is_closed[1]){
            FD_SET(sock_fd[1],&readfds);
            maxsd=((sock_fd[1]>maxsd)?sock_fd[1]:maxsd);
        }
		//printf("\\\\\\\\\\%ld\\\\\\\\\\\n",timeout.tv_sec);
		timeout.tv_sec=RETRANSMISSION_TIME;
		timeout.tv_usec=0;
        activity = select( maxsd + 1 , &readfds , NULL , NULL , &timeout);

		if(activity==0){
			//timeout occured
			printf("Timeout Occured!!\n");
			if((unacked[0]==-1)&&(unacked[1]==-1)){
				all_sent=true;
				break;
			}
			if(unacked[0]!=-1){
			send(sock_fd[0],&(pkts[unacked[0]]),sizeof(struct packet), MSG_NOSIGNAL);
			//printf("ReSending pkt %d\n",pkts[unacked[0]].seq);
            printf("SENT PKT: Seq. No %d of size %d Bytes from channel 0\n",pkts[unacked[0]].seq,pkts[unacked[0]].payload_size);
            }
			if(unacked[1]!=-1){
			send(sock_fd[1],&(pkts[unacked[1]]),sizeof(struct packet), MSG_NOSIGNAL);
			//printf("ReSending pkt %d\n",pkts[unacked[1]].seq);
            printf("SENT PKT: Seq. No %d of size %d Bytes from channel 1\n",pkts[unacked[1]].seq,pkts[unacked[1]].payload_size);
			}
		}

        for(int i=0;i<2;i++){
            if(!is_closed[i]){
                if(FD_ISSET(sock_fd[i],&readfds)){
                    if((valread=recv(sock_fd[i],&recvd_pkt,sizeof(struct packet),0))==0){
                        printf("Channel %d closed connection\n",i);
                        is_closed[i]=true;
                        close(sock_fd[i]);
                    }
                    else{ //recvd_pkt[valread]='\0';
                        //printf("Recieved packet{%d,%d,%s,%s,%s} from Channel %d\n",recvd_pkt.payload_size,recvd_pkt.seq,(recvd_pkt.is_last)?"true":"false",(recvd_pkt.is_data)?"true":"false",(recvd_pkt.channel_id)?"true":"false",i);
                        if(recvd_pkt.is_data==1){printf("Server sent a data pkt\n");}
						else{
							//printf("ACK rcvd for %d\n",recvd_pkt.seq);
							printf("RCVD ACK: for PKT with Seq. No. %d from channel %d\n",recvd_pkt.seq,i);
							if(!all_sent){
								unacked[i]=((unacked[0]<unacked[1])?unacked[1]:unacked[0])+1;
								pkts[unacked[i]].channel_id=i;
								send (sock_fd[i], &(pkts[unacked[i]]), sizeof(struct packet), MSG_NOSIGNAL);
								//printf("sending pkt %d\n",pkts[unacked[i]].seq);
                                printf("SENT PKT: Seq. No %d of size %d Bytes from channel %d\n",pkts[unacked[i]].seq,pkts[unacked[i]].payload_size,i);
								all_sent=pkts[unacked[i]].is_last;
							}
							else{
								unacked[i]=-1;
							}
						}
						memset(&recvd_pkt,0,sizeof(struct packet));
                    }
                    //FD_SET(sock_fd[0],&readfds);
                    //FD_SET(sock_fd[1],&readfds);
                }
            }
        }
		//printf("%d %d\n",unacked[0],unacked[1]);
		if(unacked[0]==-1&&unacked[1]==-1) break;   
    }   	

    close(sock_fd[0]);
    close(sock_fd[1]);
}