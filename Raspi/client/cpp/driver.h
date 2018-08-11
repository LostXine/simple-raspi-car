#ifndef DRIVER_H
#define DRIVER_H

#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>

class driver{
public:
    driver();
    ~driver();
    void setMotor(float motor);
    void setServo(float servo);
    void setMotion(float motor, float servo);
    void setDistance(unsigned long long int dist);
    void launchSpeedMode();
    void launchDistanceMode();
    void allStop();
    void getStatus();
    void getSensors();
friend void recv_threading(driver* d);
private:
    int sock;
    struct sockaddr_in serv_addr;
    int launch();
    char buf[256];
    bool keepRunning;
    std::thread recv_thread;


};

void recv_threading(driver* d);

#endif
