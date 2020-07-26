#include "packet.h"

int main(){
    srand(time(0));
    bool is_cl_set=false;
    struct sockaddr_in si_me,si_cl,si_ser,si_other;
    int slen=sizeof(si_other);
    int sock=socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP);
    if(sock==-1){printf("socket formation error\n");exit(0);}
    
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT_R1);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    si_ser.sin_family = AF_INET;
    si_ser.sin_port = htons(PORT1);
    si_ser.sin_addr.s_addr = inet_addr("127.0.0.3");

    si_cl.sin_family = AF_INET;
    si_cl.sin_port = htons(PORT_N1);
    si_cl.sin_addr.s_addr = inet_addr("127.0.0.4");
    //bind socket to port
    if( bind(sock , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        //die("bind");
        printf("binding error\n");
        exit(0);
    }
    /*
    char buf[11];
    while(1){
        recvfrom(sock,buf,10,0,(struct sockaddr *) &si_other,&slen);
        printf("%s recvd\n",buf);
    }
    */
   struct packet recvd_pkt;
    while(1){
        if(!is_cl_set){
            recvfrom(sock,&recvd_pkt,sizeof(struct packet),0,(struct sockaddr *) &si_other,&slen);
            //printf("%s recvd\n",buf);
            printf("RELAY1  R   %s  %s  %d  %s  RELAY1\n",get_time(),(recvd_pkt.is_data==false)?"ACK ":"DATA",recvd_pkt.seq,(recvd_pkt.is_data==false)?"SERVER":"CLIENT");
            //printf("Recvd %d(1 if data) pkt %d and cl address set\n",recvd_pkt.is_data,recvd_pkt.seq);
            is_cl_set=true;
        }
        else{
            recvfrom(sock,&recvd_pkt,sizeof(struct packet),0,(struct sockaddr *) &si_other,&slen);
            if(recvd_pkt.seq!=-1)
            printf("RELAY1  R   %s  %s  %d  %s  RELAY1\n",get_time(),(recvd_pkt.is_data==false)?"ACK ":"DATA",recvd_pkt.seq,(recvd_pkt.is_data==false)?"SERVER":"CLIENT");
            //printf("Recvd %d(1 if data) pkt %d\n",recvd_pkt.is_data,recvd_pkt.seq);
        }
        if(recvd_pkt.seq==-1) {printf("End of log table\n");break;}
        if(recvd_pkt.is_data==false){
            sendto(sock,&recvd_pkt,sizeof(struct packet),0,(struct sockaddr *) &si_cl,slen);
            printf("RELAY1  S   %s  ACK   %d  RELAY1  CLIENT\n",get_time(),recvd_pkt.seq);

            //printf("Ack of %d sent to client\n",recvd_pkt.seq);
        }
        else{
            int random=rand()%100;
            //printf("Random no. %d\n",random);
            if(random%100>=PDR){
            if(fork()==0){
                // srand(time(0));
                // int random=rand()%100;
                // printf("Random no. %d\n",random);
                // if(rand()%100>PDR){
                    //printf("Random no. %d\n",random);
                    sleep((rand()%3)/1000);
                    sendto(sock,&recvd_pkt,sizeof(struct packet),0,(struct sockaddr *) &si_ser,slen);
                    printf("RELAY1  S   %s  DATA  %d  RELAY1  CLIENT\n",get_time(),recvd_pkt.seq);
                    //printf("Sending %d (1 if data)pkt %d to server\n",recvd_pkt.is_data,recvd_pkt.seq);
                // }
                
                break;
            }}
            else{
                //printf("Dropping %d (1 if data) pkt %d\n",recvd_pkt.is_data,recvd_pkt.seq);
                printf("RELAY1  D   %s  DATA  %d  RELAY1  ------\n",get_time(),recvd_pkt.seq);
                continue;
            }
        }
    }
    //printf("end\n");
    close(sock);
    return 0;
}