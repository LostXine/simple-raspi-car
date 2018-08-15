/*
simple-raspi-car server
Project: https://github.com/LostXine/simple-raspi-car
Contact: lostxine@gmail.com
*/

#include "pi_serial.h"
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
#include <algorithm>
//ref: http://www.i-programmer.info/programming/cc/10027-serial-c-and-the-raspberry-pi.html?start=4

#define MAX_ERROR_COUNT 10

pi_serial::pi_serial(char* port){
    LOG(INFO)<<"Serial port online, id:"<<this<<"\033[0m"; 
    keepRunning = false;
    if(open_port(port)==0){
        err_count = 0;
        recving = new std::thread(recving_thread, this);
        sending = new std::thread(sending_thread, this);
        while(!keepRunning){usleep(1000);}
    } else {
        recving = nullptr;
        sending = nullptr;
    }
}

pi_serial::~pi_serial(){
    keepRunning = false;
    recycle_thread(sending);
    recycle_thread(recving);
    close_port();
    LOG(INFO)<<"Serial port offline, id:"<<this<<"\033[0m"; 
}

int pi_serial::open_port(char* port){
    sfd = open(port, O_RDWR | O_NOCTTY);
    if (sfd == -1) {
        LOG(FATAL)<<"No."<<errno<<" description:"<<strerror(errno);
        return -1;
    };
    struct termios options;
    tcgetattr(sfd, &options);
    bzero(&options, sizeof(options));

    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~PARENB;
    options.c_cflag |= CLOCAL;
    options.c_cflag |= CREAD;
    options.c_cflag |= CS8;
    options.c_cflag |= B115200;
    options.c_cc[VTIME] = 1;
    options.c_cc[VMIN] = 15;
    tcflush(sfd, TCIOFLUSH);
    if(tcsetattr(sfd, TCSANOW, &options)){
        LOG(FATAL)<<"No."<<errno<<" description:"<<strerror(errno);
        return -1;
    }
    return 0;
}

int pi_serial::close_port(){
    if (sfd != -1){
        close(sfd);
        return 0;
    }
    return -1;
}

int pi_serial::upload(unsigned char* buf, int size){
    try{
        unsigned char* tmp = new unsigned char[size + 1];
        tmp[0] = size;
        memcpy(tmp+1, buf, size);
        sendmutex.lock();
        sendlist.push(tmp);
        sendmutex.unlock();
    }catch(std::exception &err){
        LOG(ERROR)<<err.what();
        return -1;
    }
    return 0;
}

int pi_serial::fetch(unsigned char* buf, int size){
    try{
        recvmutex.lock();
        if(recvpool.size() < size){
            recvmutex.unlock();
            return -2;
        }
        for(int i = 0;i < size; i++){
            buf[i] = recvpool.front();
            recvpool.pop();
        }
        recvmutex.unlock();
        return 0;
    }catch(std::exception &err){
        LOG(ERROR)<<err.what();
        return -1;
    }
}

int pi_serial::get_port(int& pt){
    pt = sfd;
    return (sfd == -1)?-1:0;
}

int pi_serial::send(unsigned char* buf, int size){
    if (sfd != -1){
        auto&& log = COMPACT_GOOGLE_LOG_INFO;
        log.stream()<<"\033[0;32mSend "<<size<<" bytes: ";
        for (int i = 0; i < size; i++){
            log.stream()<<std::hex<<std::setw(2)<<std::setfill('0')<<int(buf[i])<<" ";
        }
        log.stream()<<"\033[0;0m";
        return write(sfd, buf, size);
    }else{
        return -1;
    }
}

void sending_thread(pi_serial* ps){
    LOG(INFO)<<"\033[0;32msending thread online\033[0;0m";
    while(ps->is_running()){
        if (MAX_ERROR_COUNT <= ps->err_count){
            ps->keepRunning = false;
            LOG(FATAL)<<"Serial get too much ("<< ps-> err_count <<") error.";
            break;
        }
        unsigned char* tmp = nullptr;
        ps->sendmutex.lock();
        if(!ps->sendlist.empty()){
            tmp = ps->sendlist.front();
            ps->sendlist.pop();
        }
        ps->sendmutex.unlock();
        if (tmp != nullptr){
            if (!ps->send(tmp+1, int(tmp[0]))){
                LOG(ERROR)<<"Serial sending error.";
                ps->err_count++;
            }else{
                usleep(2500);
            }
            delete [] tmp;
        }else{
            usleep(1000);
        }
    }
    // release memory
    while(!ps->sendlist.empty()){
        unsigned char* tmp = ps->sendlist.front();
        delete [] tmp;
        ps->sendlist.pop();
    }
    LOG(INFO)<<"\033[0;32msending thread offline\033[0;0m";
}

void recving_thread(pi_serial* ps){
    LOG(INFO)<<"\033[0;33mrecving thread online\033[0;0m";
    unsigned char buf[32];
    ps->keepRunning = true;
    while(ps->is_running()){
        int bytes;
        ioctl(ps->sfd, FIONREAD, &bytes);
        if (bytes != 0){
            int count = read(ps->sfd, buf, sizeof(buf));
            //auto&& log = COMPACT_GOOGLE_LOG_INFO;
            //log.stream()<<"\033[0;33mGet "<<count<<" bytes: ";
            ps->recvmutex.lock();
            for (int i = 0; i < count; i++){
                //log.stream()<<std::hex<<std::setw(2)<<std::setfill('0')<<int(buf[i])<<" ";
                ps->recvpool.push(buf[i]);
            }
            ps->recvmutex.unlock();
            //log.stream()<<"\033[0;0m";
        }
        usleep(1000);
    }
    LOG(INFO)<<"\033[0;33mrecving thread offline\033[0;0m";
}

void recycle_thread(std::thread* t){
    if (t){
        t->join();
        delete t;
        t = nullptr;
    }
};
