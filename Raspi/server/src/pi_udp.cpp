#include "pi_udp.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "rapidjson/pointer.h"

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

int pi_udp::parse_json(char* js, int len){
    rapidjson::Document d;
    d.Parse(js);
    if (d.HasParseError()) {return -1;}
try{
    //check uid
    unsigned long long int tmp = 0;
    if (rapidjson::Value* tmd = rapidjson::Pointer("/uid").Get(d)){
    tmp = tmd->GetDouble();
    }
    if (tmp < uid){
         LOG(WARNING)<<"Invalid timestamp: "<< js; 
         return -2;
    }else{
        uid = tmp;
    }
    float set_motor = s_motor;
    float set_servo = s_servo;
    unsigned long long int set_dist = s_dist;
    //check weither to send;
    int to_send = 0;
    rapidjson::Value* var;
    //check speed
    if (var = rapidjson::Pointer("/sm").Get(d)){
        set_motor = var->GetDouble();
        s_motor = set_motor;
        to_send++;
    }

    //check servo
    if (var = rapidjson::Pointer("/ss").Get(d)){
        set_servo = var->GetDouble();
        s_servo = set_servo;
        to_send++;
    }
    //set speed and motion
    if (to_send){
        driver->set_motion(set_motor, set_servo);
        to_send = 0;
    }

    //check distance
    if (var = rapidjson::Pointer("/sd").Get(d)){
        set_dist = var->GetUint64();
        s_dist = set_dist;
        driver->set_distance_num(set_dist);
        to_send = 0;
    }

    //check mode
    if (var = rapidjson::Pointer("/so").Get(d)){
        int po = var->GetInt();
        char option = 0x03 & po;
        driver->set_status(option);
    }

    //check query requests
    if (var = rapidjson::Pointer("/ro").Get(d)){
        driver->query_status();
    }
    if (var = rapidjson::Pointer("/rs").Get(d)){
        driver->query_sensor();
    }
    
    return 0;
}
catch(exception &err){
LOG(ERROR)<<err.what()<<endl;
    return 1;
}
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
            if (pu->parse_json(buf, recv_num)){
                LOG(WARNING)<<"Parse error: " <<buf;
            }
        }
        usleep(1000);
    }
    LOG(INFO)<<"\033[0;35mlistening thread offline\033[0;0m";
}
