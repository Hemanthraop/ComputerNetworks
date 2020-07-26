#include "packet.h"

int main(){
    struct sockaddr_in si_me1,si_me2, si_r1,si_r2,si_other;
    int slen=sizeof(si_r1);

    int sock_r1=socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP);
    if(sock_r1==-1){printf("socket formation error\n");exit(0);}

    int sock_r2=socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP);
    if(sock_r2==-1){printf("socket formation error\n");exit(0);}

    si_me1.sin_family = AF_INET;
    si_me1.sin_port = htons(PORT1);
    si_me1.sin_addr.s_addr = htonl(INADDR_ANY);

    if( bind(sock_r1 , (struct sockaddr*)&si_me1, sizeof(si_me1) ) == -1)
    {
        printf("binding error for sock_r1\n");
        exit(0);
    }
    
    si_me2.sin_family = AF_INET;
    si_me2.sin_port = htons(PORT2);
    si_me2.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if( bind(sock_r2 , (struct sockaddr*)&si_me2, sizeof(si_me2) ) == -1)
    {
        printf("binding error for sock_r2\n");
        exit(0);
    }

    si_r1.sin_family = AF_INET;
    si_r1.sin_port = htons(8888);
    si_r1.sin_addr.s_addr = inet_addr("127.0.0.1");
    /*
    char buf[11];
    strcpy(buf,"serr1");
    slen=sizeof(si_r1);
    sendto(sock_r1,buf,6,0, (struct sockaddr *) &si_r1,slen);
    */
    si_r2.sin_family = AF_INET;
    si_r2.sin_port = htons(8889);
    si_r2.sin_addr.s_addr = inet_addr("127.0.0.2");
    /*
    //char buf[11];
    strcpy(buf,"serr2");
    slen=sizeof(si_r2);
    sendto(sock_r2,buf,6,0, (struct sockaddr *) &si_r2,slen);
    */
    int valread;
    
    char buffer[WIND_SIZE*PACKET_SIZE];
    int wstart=0;
    fd_set readfds;
    FD_ZERO(&readfds);
    // FD_SET(sock_r1,&readfds);
    // FD_SET(sock_r2,&readfds);
    int maxsd=(sock_r1<sock_r2)?sock_r2:sock_r1;
    int activity;
    int sock_fd[2];
    struct packet recvd_pkt;
    sock_fd[0]=sock_r1;
    sock_fd[1]=sock_r2;
    FD_SET(sock_fd[0],&readfds);
    //FD_SET(sock_fd[1],&readfds);
    //int wstart=0;
    bool isbuf[WIND_SIZE]={false};
    FILE * fp=fopen("output.txt","w");
    bool last_came=false,tmp=false;
    while(1){
        //printf("%d %d %d %d\n",isbuf[0],isbuf[1],isbuf[2],isbuf[3]);
        //printf("%d\n",last_came);
        tmp=false;
        for(int i=0;i<WIND_SIZE;i++){
            tmp=((tmp)||(isbuf[i]));
        }
        if((last_came)&&(!tmp)){
            break;
        }
        activity=select(maxsd+1,&readfds,NULL,NULL,NULL);
        //printf("Activity:%d\n",activity);
        for(int i=0;i<1;i++){
            if(FD_ISSET(sock_fd[i],&readfds)){
                if((valread=recvfrom(sock_fd[i],&recvd_pkt,sizeof(struct packet),0,(struct sockaddr *)&si_other,&slen))==0){
                    printf("Channel %d Closed\n",i);
                }
                else{
                    //this is important
                    //printf("Recvd %d(1 if data) pkt %d\n",recvd_pkt.is_data,recvd_pkt.seq);
                    printf("SERVER  R   %s  DATA  %d  %s  SERVER\n",get_time(),recvd_pkt.seq,(recvd_pkt.seq%2==0)?"RELAY2":"RELAY1");
                    if(recvd_pkt.is_last) last_came=true;
                    if(recvd_pkt.seq==wstart){
                        for(int j=0;j<PACKET_SIZE;j++){
                            buffer[j]=recvd_pkt.payload[j];
                            //printf("%c",buffer[j]);
                        }
                        //printf("\n");
                        isbuf[0]=true;
                        for(int j=0;j<WIND_SIZE;j++){
                            if(isbuf[0]==true){
                                for(int k=0;k<PACKET_SIZE;k++){
                                    fputc(buffer[k],fp);
                                    
                                }
                                //printf("wrote to file\n");
                                wstart=(wstart+1)%(2*WIND_SIZE);
                                for(int k=0;k<WIND_SIZE-1;k++){
                                    isbuf[k]=isbuf[k+1];
                                }
                                isbuf[WIND_SIZE-1]=false;
                                for(int k=0;k<WIND_SIZE-1;k++){
                                    for(int l=0;l<PACKET_SIZE;l++){
                                        buffer[PACKET_SIZE*k+l]=buffer[PACKET_SIZE*(k+1)+l];
                                    }
                                }
                            }
                            else{
                                // printf("%d %d %d %d\n",isbuf[0],isbuf[1],isbuf[2],isbuf[3]);
                                break;
                            }
                        }
                        recvd_pkt.is_data=false;
                        sendto(sock_r1,&recvd_pkt,sizeof(struct packet),0,(struct sockaddr *) &si_r1,slen);
                        printf("SERVER  S   %s  ACK   %d  SERVER  RELAY1\n",get_time(),recvd_pkt.seq);
                        //printf("Ack of %d sent to r1\n",recvd_pkt.seq);
                    }
                    else{
                        if(recvd_pkt.seq<wstart){
                            isbuf[recvd_pkt.seq+2*WIND_SIZE-wstart]=true;
                            for(int j=0;j<PACKET_SIZE;j++){
                                buffer[(recvd_pkt.seq+2*WIND_SIZE-wstart)*PACKET_SIZE+j]=recvd_pkt.payload[j];
                                //printf("%c",buffer[(recvd_pkt.seq+2*WIND_SIZE-wstart)*PACKET_SIZE+j]);
                            }
                            //printf("\n");
                        }
                        else{
                            isbuf[recvd_pkt.seq-wstart]=true;
                            for(int j=0;j<PACKET_SIZE;j++){
                                buffer[(recvd_pkt.seq-wstart)*PACKET_SIZE+j]=recvd_pkt.payload[j];
                                //printf("%c",buffer[(recvd_pkt.seq-wstart)*PACKET_SIZE+j]);
                            }
                            //printf("\n");
                        }
                        recvd_pkt.is_data=false;
                        sendto(sock_r1,&recvd_pkt,sizeof(struct packet),0,(struct sockaddr *) &si_r1,slen);
                        printf("SERVER  S   %s  ACK   %d  SERVER  RELAY1\n",get_time(),recvd_pkt.seq);
                        //printf("Ack of %d sent to r2\n",recvd_pkt.seq);
                    }
                }
            }
        }
    }

    fclose(fp);
}