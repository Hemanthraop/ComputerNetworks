#include "packet.h"

bool is_closed[2];

void write_to_file(struct packet pkt,FILE * fp){
    //printf("writing pkt %d to file\n",pkt.seq);
    for(int i=0;i<pkt.payload_size;i++){
        fputc((pkt.payload)[i],fp);
        //printf("%c",(pkt.payload)[i]);
    }
    //printf("\n");
}

void copy_to(struct packet * pktptr,struct packet pkt){
    pktptr->channel_id=pkt.channel_id;
    pktptr->is_data=pkt.is_data;
    pktptr->is_last=pkt.is_last;
    pktptr->seq=pkt.seq;
    pktptr->payload_size=pkt.payload_size;
    for(int i=0;i<pkt.payload_size;i++)
        pktptr->payload[i]=pkt.payload[i];
}

int main(){
    /*char ch;
    FILE* fp=fopen("inut.txt","r");
    struct packet * pkts;
    fseek(fp,0,2);
    int file_size=ftell(fp);
    fclose(fp);

    while(!feof(fp)){
        ch=fget(fp,"%c",&())
    }
    */


    int server_sock_fd[2];
    int sock_fd[2];
    struct sockaddr_in serverAddress[2];
    struct sockaddr_in clientAddress[2];
    int tmp;
    int clientLength[2];
    int opt=1;
    for(int i=0;i<2;i++){
        server_sock_fd[i] = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
        //printf("Socket %d created\n",sock_fd[i]);
        setsockopt(server_sock_fd[i], SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt));
        memset (&(serverAddress[i]), 0, sizeof(serverAddress[i]));
        serverAddress[i].sin_family = AF_INET;
        serverAddress[i].sin_port = htons((i==0)?PORT1:PORT2);
        serverAddress[i].sin_addr.s_addr = htonl(INADDR_ANY);

        tmp=bind(server_sock_fd[i],(struct sockaddr*) &(serverAddress[i]),sizeof(serverAddress[i]));
        //printf("binded\n");
        tmp=listen(server_sock_fd[i],20);
        //printf("listening\n");
        clientLength[i]=sizeof(clientAddress[i]);
        sock_fd[i]=accept(server_sock_fd[i],(struct sockaddr*) &(clientAddress[i]),&clientLength[i]);
        //printf("Socket recieved %d\n",sock_fd[i]);
        //printf("Port: %d\n",ntohs(serverAddress[i].sin_port));
        close(server_sock_fd[i]);
    }

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sock_fd[0],&readfds);
    FD_SET(sock_fd[1],&readfds);
    int drop_count=0;

    int activity;// = select( ((sock_fd[0]<sock_fd[1])?sock_fd[1]:sock_fd[0]) + 1 , &readfds , NULL , NULL , NULL);
    int valread;
    struct packet recvd_pkt;
    int maxsd;
    int pdr=PDR;
    pdr=(PDR>=50)?50:PDR;
    //for(int j=0;j<4;j++)
    bool writing_to_file=true,writing_to_buffer=false;
    int bufflen=0,initial_seq,latest=-1;
    struct packet buffer[1000];
    FILE* fp=fopen("output.txt","w");

    int iter=0;
    while(1){
        iter++;
        //printf("iter: %d\n %d %d\n",iter,is_closed[0],is_closed[1]);
        FD_ZERO(&readfds);
        
        if(is_closed[0]&&is_closed[1]){
            //printf("Here\n");
            //printf("%d\n",bufflen);
            if(bufflen!=0){
                for(int i=0;i<bufflen;i++){
                    write_to_file(buffer[i],fp);
                }
            }
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

        activity = select( maxsd + 1 , &readfds , NULL , NULL , NULL);

        for(int i=0;i<2;i++){
            if(!is_closed[i]){
                if(FD_ISSET(sock_fd[i],&readfds)){
                    if((valread=recv(sock_fd[i],&recvd_pkt,sizeof(struct packet),0))==0){
                        //printf("Channel %d closed connection\n",i);
                        is_closed[i]=true;
                        close(sock_fd[i]);
                    }
                    else{ //recvd_pkt[valread]='\0';
                        //printf("Recieved packet{%d,%d,%s,%s,%s} from Channel %d\n",recvd_pkt.payload_size,recvd_pkt.seq,(recvd_pkt.is_last)?"true":"false",(recvd_pkt.is_data)?"true":"false",(recvd_pkt.channel_id)?"true":"false",i);
                        recvd_pkt.is_data=false;
                        if(drop_count!=(100/pdr)-1){
                            //printf("Recieved packet{%d,%d,%s,%s,%s} from Channel %d\n",recvd_pkt.payload_size,recvd_pkt.seq,(recvd_pkt.is_last)?"true":"false",(recvd_pkt.is_data)?"true":"false",(recvd_pkt.channel_id)?"true":"false",i);
                            printf("RCVD PKT: Seq. No %d of size %d Bytes from channel %d\n",recvd_pkt.seq,recvd_pkt.payload_size,recvd_pkt.channel_id);
                            //write in file or a buffer
                            //start

                            if((writing_to_file)&&(recvd_pkt.seq==(latest+1))){
                                //printf("1\n");
                                write_to_file(recvd_pkt,fp);
                                latest++;
                            }
                            else if((writing_to_file)&&(recvd_pkt.seq!=(latest+1))){

                                //copy the whole recvd_pkt to buffer
                                //printf("2\n");
                                copy_to(&(buffer[recvd_pkt.seq-latest-1]),recvd_pkt);
                                initial_seq=latest+1;
                                writing_to_file=false;
                                writing_to_buffer=true;
                                latest=recvd_pkt.seq;
                                bufflen++;
                            }
                            else if((writing_to_buffer)&&(recvd_pkt.seq==(latest+1))){
                                //printf("3\n");
                                copy_to(&(buffer[latest+1-initial_seq]),recvd_pkt);
                                latest++;
                                bufflen++;
                            }
                            else if((writing_to_buffer)&&(recvd_pkt.seq==initial_seq)){
                                //printf("4\n");
                                copy_to(&(buffer[0]),recvd_pkt);
                                bufflen++;
                                //printf("%d\n",bufflen);
                                for(int i=0;i<bufflen;i++){
                                    write_to_file(buffer[i],fp);
                                }
                                bufflen=0;
                                writing_to_file=true;
                                writing_to_buffer=false;
                                //don't change latest here
                            }


                            //end
                            send(sock_fd[i],&recvd_pkt,sizeof(struct packet),MSG_NOSIGNAL);
                            //printf("Ack sent for pkt %d\n",recvd_pkt.seq);
                            printf("SENT ACK: for PKT with Seq. No. %d from Channel %d\n",recvd_pkt.seq,recvd_pkt.channel_id);
                            drop_count++;
                        }
                        else{
                            //printf("Dropping packet{%d,%d,%s,%s,%s} from Channel %d\n",recvd_pkt.payload_size,recvd_pkt.seq,(recvd_pkt.is_last)?"true":"false",(recvd_pkt.is_data)?"true":"false",(recvd_pkt.channel_id)?"true":"false",i);
                            //dropping packet
                            drop_count=0;
                        }
                        memset(&recvd_pkt,0,sizeof(struct packet));
                    }
                    //FD_SET(sock_fd[0],&readfds);
                    //FD_SET(sock_fd[1],&readfds);
                }
            }
        }   
    }
    /*else if(FD_ISSET(sock_fd[1],&readfds)){
        printf("Recieved a message from Channel 1\n");
        FD_SET(sock_fd[0],&readfds);
        FD_SET(sock_fd[1],&readfds);
    }*/
    /*close(sock_fd[0]);
    close(sock_fd[1]);*/
    sleep(1);
    fclose(fp);
}