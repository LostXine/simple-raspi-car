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
        void setCamPitch(float pitch);
        void setCamYaw(float yaw);
        void launchVoltageMode();
        void allStop();
        void getMode();
        void getMotor();
        void getServo();
        void getCamPitch();
        void getCamYaw();
        
    friend void recv_threading(driver* d);

    private:
        int sock;
        struct sockaddr_in serv_addr;
        int launch();
        void set_code(const char* code, float v);
        void query_code(int code);
        char buf[256];
        bool keepRunning;
        std::thread recv_thread;
};

void recv_threading(driver* d);
#endif
