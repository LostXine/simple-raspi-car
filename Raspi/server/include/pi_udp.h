/*
simple-raspi-car server
Project: https://github.com/LostXine/simple-raspi-car
Contact: lostxine@gmail.com
*/

#ifndef PI_UDP_H
#define PI_UDP_H

#include "pi_driver.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

class pi_udp{
public:
    pi_udp(pi_driver* pd);
    ~pi_udp();
    bool is_running(){return keepRunning && driver->is_running();}

friend void listening_thread(pi_udp* pu);
    int send(char* buf, int len);

private:
    bool keepRunning;
    pi_driver* driver;
    int udps; //udp socket
    int start_listen(int port);
    int finish_listen();
    std::thread* listening;

    struct sockaddr client_addr;
    unsigned int client_len = 0;
};

void listening_thread(pi_udp* pu);

#endif