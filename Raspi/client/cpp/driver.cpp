#include "driver.h"
#include <unistd.h>
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>

using namespace std;

driver::driver(){
    printf("----Driver ID:%d Init----\n", this);

    sock = socket(PF_INET, SOCK_DGRAM, 0);
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(61551);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);
    
    keepRunning = true;
    recv_thread = thread(recv_threading, this);
}

driver::~driver(){
    keepRunning = false;
    recv_thread.join();
    printf("----Driver ID:%d End----\n", this);
}

void driver::set_code(char* code, float v){
    int value = max(min(1.0f, v), -1.0f) * 100;
    sprintf(buf, "{\"%s\":%d}", code, value);
    launch();
}

void driver::setMotor(float motor){
    set_code("sm", motor);
}

void driver::setServo(float servo){
    set_code("ss", motor);
}

void driver::setCamPitch(float pitch){
    set_code("scp", pitch);
}

void driver::setCamYaw(float yaw){
    set_code("scy", yaw);
}

void driver::setMotion(float motor, float servo){
    motor = max(min(1.0f, motor), -1.0f);
    servo = max(min(1.0f, servo), -1.0f);
    sprintf(buf, "{\"sm\":%d, \"ss\":%d}",int(motor * 100), int(servo * 100));
    launch();
}

void drvier::query_code(int code){
    sprintf(buf, "{\"rq\":%d}", code);
    launch();
}

void driver::getMode(){
    query_code(0);
}

void driver::getMotor(){
    query_code(1);
}

void driver::getServo(){
    query_code(2);
}

void getCamPitch(){
    query_code(3);
}

void getCamYaw(){
    query_code(4);
}

void driver::launchVoltageMode(){
    sprintf(buf, "{\"so\":1}");
    launch();
}

void driver::allStop(){
    sprintf(buf, "{\"so\":0}");
    launch();
}

int driver::launch(){
    if (sendto(sock, buf, strlen(buf), 0, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in))< 0){
        printf("sendto failed.\n");
        return 1;
    }
    return 0;
}

void recv_threading(driver* d){
   char buf[256];
   struct sockaddr client_addr;
   socklen_t client_len = sizeof(client_addr);
   while(d->keepRunning){
        int recv_num = recvfrom(d->sock,buf,sizeof(buf),0,&client_addr, &client_len);
        if (recv_num > 0){
            if(recv_num < sizeof(buf)){
                buf[recv_num] = 0;
            };
            printf("recv: %s\n", buf);
        }
        usleep(1000);
   }
}