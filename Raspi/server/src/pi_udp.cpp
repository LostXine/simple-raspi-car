#include "pi_udp.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define UDP_PORT     61551
#define RECV_BUF_LEN 256

using namespace std;

pi_udp::pi_udp(pi_driver* pd){
    LOG(INFO)<<"UDP manager online, id:"<<this<<"\033[0m"; 
    keepRunning = false;
    bzero(&client_addr, sizeof(client_addr));
    driver = pd;
    pd->set_udp(this);
    udps = -1;
    if (!start_listen(UDP_PORT)){
        listening = new std::thread(listening_thread, this);
        while(!keepRunning){usleep(1000);}
    } else {
        listening = nullptr;
    }
}

pi_udp::~pi_udp(){
    keepRunning = false;
    recycle_thread(listening);
    if (udps!=-1){close(udps);}
    LOG(INFO)<<"UDP manager offline, id:"<<this<<"\033[0m"; 
}

int pi_udp::start_listen(int port){
    struct sockaddr_in  localAddr;
    //AF_UNIX:localhost AF_INET:ipv4 AF_INET6:ipv6
    //SOCK_STREAM:TCP SOCK_DGRAM:UDP SOCK_RAW:raw
    udps = socket(AF_INET, SOCK_DGRAM, 0); 
    if (udps == -1){
        LOG(ERROR)<<"No."<<errno<<" description:"<<strerror(errno);
        return -1;
    }
    //set to non-block
    fcntl(udps, F_SETFL, fcntl(udps, F_GETFL, 0) | O_NONBLOCK);
    bzero(&localAddr, sizeof(localAddr));
    //config local server network
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(port);
    localAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(udps, (struct sockaddr *)&localAddr, sizeof(localAddr))){
        LOG(ERROR)<<"No."<<errno<<" description:"<<strerror(errno);
        return -1;
    }
    return 0;
}



int pi_udp::send(char* buf, int len){
    if (udps==-1){
        LOG(ERROR)<<"No."<<errno<<" description:"<<strerror(errno);
        return -3;
    }
    if (client_addr.sa_data[0] == 0 && client_addr.sa_data[1] == 0){
        LOG(WARNING)<<"No UDP client found.";
        return -2;
    }
    if(!sendto(udps, buf, len, 0, &client_addr, client_len)){
        return -1;
    }else{
        LOG(INFO)<<"Transfer "<< strlen(buf) <<" bytes: "<< buf;
        return 0;
    }
}

void listening_thread(pi_udp* pu){
    LOG(INFO)<<"\033[0;35mlistening thread online\033[0;0m";
    pu->keepRunning = true;
    char buf[RECV_BUF_LEN];
    while(pu->is_running()){
        int recv_num = recvfrom(pu->udps,buf,RECV_BUF_LEN,0,&(pu->client_addr),&(pu->client_len));
        if (recv_num > 0){
            if(recv_num < RECV_BUF_LEN){
                buf[recv_num] = 0;
            };
            auto&& log = COMPACT_GOOGLE_LOG_INFO;
            log.stream()<<"\033[0;35mFrom ";
            for(int i = 0; i < 4; i++){
                log.stream()<<int(pu->client_addr.sa_data[i+2]);
                if(i!=3){log.stream()<<".";}
            }
            int port = pu->client_addr.sa_data[0];
            port = (port << 8) + pu->client_addr.sa_data[1];
            log.stream()<<"("<< port <<"): "<<buf<<"\033[0;0m";
            //.sa_data: [0-1]: port [2-5]:ip
            //for (int i = 0; i < 14; i++){printf("%d.", int(pu->client_addr.sa_data[i]));}
            //printf("\n");
            if (pu->pi_driver->parse_json(buf, recv_num)){
                LOG(WARNING)<<"Parse error: " <<buf;
            }
        }
        usleep(1000);
    }
    LOG(INFO)<<"\033[0;35mlistening thread offline\033[0;0m";
}
