/*
simple-raspi-car server
Project: https://github.com/LostXine/simple-raspi-car
Contact: lostxine@gmail.com
*/

#ifndef PI_SERIAL_H
#define PI_SERIAL_H

#include<thread>
#include<queue>
#include<mutex>
#include<unistd.h>
#include<iomanip>
#include<exception>

#include"glog/logging.h" 

class pi_serial{

    public:
        pi_serial(char* port);
        ~pi_serial();

        //get serial port
        int get_port(int& pt);
        int open_port(char* port);
        int close_port();

        //check status of port
        bool is_running(){return keepRunning;}

        int upload(char* buf, int size);
        int fetch(char* buf, int size);

        friend void sending_thread(pi_serial* ps);
        friend void recving_thread(pi_serial* ps);

    private:
        int sfd;
        int err_count;
        bool keepRunning;
        std::queue<char> recvpool;
        std::queue<char*> sendlist;
        std::mutex recvmutex;
        std::mutex sendmutex;
        std::thread* sending;
        std::thread* recving;
        // send data to serial port
        int send(char* buf, int size);
};

void sending_thread(pi_serial* ps);
void recving_thread(pi_serial* ps);

void recycle_thread(std::thread* t);

#endif


