/*
simple-raspi-car server
Project: https://github.com/LostXine/simple-raspi-car
Contact: lostxine@gmail.com
*/

#ifndef PI_DRIVER_H
#define PI_DRIVER_H

#include "pi_serial.h"

#define START_FLAG 0xff
#define END_FLAG   0xaf

typedef int (*SDCB)(char*, int);

class pi_driver{
    public:
        pi_driver(pi_serial* ps);
        ~pi_driver();
        void set_port(pi_serial* ps);

        bool is_running(){return keepRunning && port->is_running();}
        // var :    1 ---- 0 ---- -1
        //motor: forward  stop  backward
        //servo:  left    mid    right 
        void set_motion(float motor, float servo);// two motion :motor and servo
        void set_distance(unsigned char * dt);// set distance
        void set_distance_num(unsigned long long dt);// set distance

        // option: bit 3 and bit 2
        void launch_speed_mode(char option = 0);    // -> set_status
        void launch_distance_mode(char option = 0); // -> set_status
        void stop_any_mode(char option = 0);        // -> set_status
        // bit |    3     2         1       0
        //  0  |   N/A   N/A   speed mode  stop
        //  1  |  reset reset  distance    run 
        void set_status(char status);

        //query status
        void query_status();
        void query_sensor();

        //set udp_send_back
        void set_udp(void* _udp);

        unsigned int get_CRC16(char* src, int len);
        void fill_CRC16(char* buf, int len);

        friend void fetching_thread(pi_driver* pd);

    private:
        pi_serial* port;
        bool keepRunning;
        int err_count;
        std::queue<int>tofetchlist; // int<0: need to parse else: do nothing
        std::mutex fetchmutex;
        std::thread* fetching;
        void launch_msg(char* src, int len, int tofetch);
        void transfer_response(char* src, int len);
        void* udp = nullptr;
        
};


void fetching_thread(pi_driver* pd);

#endif
