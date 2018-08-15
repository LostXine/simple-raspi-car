/*
simple-raspi-car server
Project: https://github.com/LostXine/simple-raspi-car
Contact: lostxine@gmail.com
*/

#ifndef PI_DRIVER_H
#define PI_DRIVER_H

#include "pi_serial.h"

#define START_FLAG 0xFF   // start flag
#define END_FLAG   0xAF   // end flag

class pi_driver{
    public:
        pi_driver(pi_serial* ps);
        ~pi_driver();
        void set_port(pi_serial* ps);

        bool is_running(){return keepRunning && port->is_running();}
        // set
        void set_mode(char v);
        // var :    -1 ---- 0 ---- 1
        //motor: backward  stop  forward
        //servo:   left    mid   right
        void set_motor(int v);
        void set_servo(int v);

        // query
        void query_mode();
        void query_motor();
        void query_servo();
        
        //set udp_send_back
        void set_udp(void* _udp);
        // parse json
        int parse_json(char* js, int len);

        unsigned int get_CRC16(unsigned char* src, int start, int fin);
        void fill_CRC16(unsigned char* src, int start, int fin);

        friend void fetching_thread(pi_driver* pd);

    private:
        pi_serial* port;
        bool keepRunning;
        int err_count;
        std::queue<int>tofetchlist; // int<0: need to parse else: do nothing
        std::mutex fetchmutex;
        std::thread* fetching;
        void launch_msg(unsigned char* src, int len, int tofetch);
        void transfer_response(unsigned char* src, int fin);
        void set_cmd(unsigned char code, int v);
        unsigned int assemble_bytes(unsigned char a, unsigned char b);
        void* udp = nullptr;
        
};


void fetching_thread(pi_driver* pd);

#endif
