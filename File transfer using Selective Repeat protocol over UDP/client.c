
#include "packet.h"

int main(){
    
    
    //Firstly, Dividing file and preparing chunks

    FILE* fp=fopen("input.txt","r");
    
    //fseek(fp,0,2);
    fseek(fp, 0L, SEEK_END);
    int file_size=ftell(fp)-1;
    //printf("File size: %d\n",file_size);
    fclose(fp);
    
    int total_no_of_pkts;
    if(file_size%PACKET_SIZE==0)
        total_no_of_pkts=file_size/PACKET_SIZE;
    else
        total_no_of_pkts=(file_size/PACKET_SIZE)+1;
    
    char chunks [total_no_of_pkts][PACKET_SIZE];
    memset(chunks,'\0',sizeof(chunks));
    fp=fopen("input.txt","r");
    int curr=-1,offset=PACKET_SIZE;
    for(int i=0;i<file_size;i++){
         if(offset==PACKET_SIZE){
            offset=0;
            curr++;
        }
        fscanf(fp,"%c",&(chunks[curr][offset]));
        offset++;
    }
    fclose(fp);
    //commented code for printing 
    /*
    for(int i=0;i<total_no_of_pkts;i++){
        for(int j=0;j<PACKET_SIZE;j++){
            printf("%c",chunks[i][j]);
        }
        printf("\n");
    }
    */ 
    
    
    int sock_r1=socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP);
    if(sock_r1==-1){printf("socket formation error\n");exit(0);}

    struct sockaddr_in si_r1,si_r2,si_other,si_cl;
    int slen=sizeof(si_r1);

    memset((char *) &si_r1,0,sizeof(si_r1));
    memset((char *) &si_r2,0,sizeof(si_r2));
    memset((char *) &si_other,0,sizeof(si_other));

    si_r1.sin_family = AF_INET;
    si_r1.sin_port = htons(PORT_R1);
    si_r1.sin_addr.s_addr = inet_addr("127.0.0.1");

    si_r2.sin_family = AF_INET;
    si_r2.sin_port = htons(PORT_R2);
    si_r2.sin_addr.s_addr = inet_addr("127.0.0.2");

    si_cl.sin_family = AF_INET;
    si_cl.sin_port = htons(PORT_N1);
    si_cl.sin_addr.s_addr = ntohl(INADDR_ANY);

    bind(sock_r1 , (struct sockaddr*)&si_cl, sizeof(si_cl));

    struct packet copy[2*WIND_SIZE];
    struct packet pkt;
    int chunk_no=-1;
    bool allsent=false;
    int wstart=0,wend=0;
    for(int i=0;i<WIND_SIZE;i++){
        if(!allsent){
            wend=i;
            chunk_no+=1;
            pkt.seq=chunk_no%(2*WIND_SIZE);
            pkt.is_data=true;
            pkt.payload_size=PACKET_SIZE;
            if(chunk_no==total_no_of_pkts-1){
                pkt.is_last=true;
            }
            else{
                pkt.is_last=false;
            }
            for(int j=0;j<pkt.payload_size;j++){
                pkt.payload[j]=chunks[chunk_no][j];
            }
            sendto(sock_r1,&pkt,sizeof(struct packet),0,(struct sockaddr *) ((i%2==0)?&si_r2:&si_r1),slen);
            printf("CLIENT  S   %s  DATA  %d  CLIENT  %s\n",get_time(),i,(i%2==0)?"RELAY2":"RELAY1");
            copy[pkt.seq].seq=pkt.seq;
            copy[pkt.seq].is_data=pkt.is_data;
            copy[pkt.seq].is_last=pkt.is_last;
            copy[pkt.seq].payload_size=pkt.payload_size;
            for(int j=0;j<pkt.payload_size;j++){
                copy[pkt.seq].payload[j]=pkt.payload[j];
            }

            allsent=pkt.is_last;
        }
    }
    
    fd_set readfds;
    FD_ZERO(&readfds);
    //FD_SET(sock_r1,&readfds);
    //FD_SET(sock_fd[1],&readfds);

	struct timeval timeout;
	timeout.tv_sec=RETRANSMISSION_TIME;  
	timeout.tv_usec=0;
    int activity;
    int maxsd=sock_r1;
    struct packet recvd_pkt;
    bool acks_rcvd[WIND_SIZE]={false};
    bool tmp=false;
    while (1)
    {   
        FD_SET(sock_r1,&readfds);  
        tmp=true;
        for(int i=wstart;i<=((wstart<=wend)?wend:(wend+(2*WIND_SIZE)));i++){
            tmp=(tmp)&&(acks_rcvd[i-wstart]);
        }
        if(tmp) break;

        timeout.tv_sec=RETRANSMISSION_TIME;
		timeout.tv_usec=0;
        activity=select(maxsd+1,&readfds,NULL,NULL,&timeout);

        if(activity==0){
            //timeout occured
            //printf("timeout occured\n");
            printf("CLIENT  TO  %s\n",get_time());
            for(int i=wstart;i<=((wstart<=wend)?wend:(wend+(2*WIND_SIZE)));i++){
                if(acks_rcvd[i-wstart]==false){
                    pkt.seq=copy[i%(2*WIND_SIZE)].seq;
                    pkt.is_data=true;
                    pkt.payload_size=PACKET_SIZE;
                    
                    pkt.is_last=copy[i%(2*WIND_SIZE)].is_last;
                    
                    for(int j=0;j<pkt.payload_size;j++){
                        pkt.payload[j]=copy[i%(2*WIND_SIZE)].payload[j];
                    }
                    sendto(sock_r1,(struct packet *)&pkt,sizeof(struct packet),0,(struct sockaddr *) (((copy[i%(2*WIND_SIZE)].seq)%2==0)?&si_r2:&si_r1),slen);
                    printf("CLIENT  RE  %s  DATA  %d  CLIENT  %s\n",get_time(),copy[i%(2*WIND_SIZE)].seq,((copy[i%(2*WIND_SIZE)].seq)%2==0)?"RELAY2":"RELAY1");
                }
            }
        }
        else{
            recvfrom(sock_r1,&recvd_pkt,sizeof(struct packet),0,(struct sockaddr *) &si_other,&slen);
            // printf("CLIENT  R   %s  ACK  %d  %s  CLIENT\n",get_time(),copy[i%(2*WIND_SIZE)].seq,((copy[i%(2*WIND_SIZE)].seq)%2==0)?"RELAY2":"RELAY1");
            
            if(recvd_pkt.is_data==true){printf("cl recvd data pkt\n");}
            else{
                //printf("Ack for %d pkt rcvd\n",recvd_pkt.seq);
                if(recvd_pkt.seq==wstart){
                    printf("CLIENT  R   %s  ACK   %d  RELAY1  CLIENT\n",get_time(),recvd_pkt.seq);
                    if(allsent){
                        acks_rcvd[0]=true;
                        continue;
                    }
                    for(int i=0;i<WIND_SIZE;i++){
                        /*if(allsent){
                            acks_rcvd[0]=true;
                            break;
                        }*/
                        wstart++;
                        wstart=wstart%(2*WIND_SIZE);
                        wend++;
                        wend=wend%(2*WIND_SIZE);

                        chunk_no+=1;
                        pkt.seq=chunk_no%(2*WIND_SIZE);
                        pkt.is_data=true;
                        pkt.payload_size=PACKET_SIZE;
                        if(chunk_no==total_no_of_pkts-1){
                            pkt.is_last=true;
                        }
                        else{
                            pkt.is_last=false;
                        }
                        for(int j=0;j<pkt.payload_size;j++){
                            pkt.payload[j]=chunks[chunk_no][j];
                        }
                        sendto(sock_r1,&pkt,sizeof(struct packet),0,(struct sockaddr *) ((chunk_no%2==0)?&si_r2:&si_r1),slen);
                        printf("CLIENT  S   %s  DATA  %d  CLIENT  %s\n",get_time(),pkt.seq,(chunk_no%2==0)?"RELAY2":"RELAY1");

                        copy[pkt.seq].seq=pkt.seq;
                        copy[pkt.seq].is_data=pkt.is_data;
                        copy[pkt.seq].is_last=pkt.is_last;
                        copy[pkt.seq].payload_size=pkt.payload_size;
                        for(int j=0;j<pkt.payload_size;j++){
                            copy[pkt.seq].payload[j]=pkt.payload[j];
                        }

                        allsent=pkt.is_last;

                        for(int j=0;j<WIND_SIZE-1;j++){
                        acks_rcvd[j]=acks_rcvd[j+1];
                        }
                        acks_rcvd[WIND_SIZE-1]=false;

                        if(allsent) break;
                        if(acks_rcvd[0]==false) break;
                    }
                }
                else{
                    //ack of middle pkt rcvd
                    printf("CLIENT  R   %s  ACK   %d  RELAY1  CLIENT\n",get_time(),recvd_pkt.seq);

                    if(recvd_pkt.seq<wstart){
                        acks_rcvd[recvd_pkt.seq+(2*WIND_SIZE)-wstart]=true;
                    }
                    else{
                        acks_rcvd[recvd_pkt.seq-wstart]=true;
                    }
                }
            }
        }
        //FD_SET(sock_r1,&readfds); 
    }
    pkt.seq=-1;
    sendto(sock_r1,&pkt,sizeof(struct packet),0,(struct sockaddr *)&si_r1,slen);
    sendto(sock_r1,&pkt,sizeof(struct packet),0,(struct sockaddr *)&si_r2,slen);
}