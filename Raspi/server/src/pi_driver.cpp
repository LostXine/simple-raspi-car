/*
simple-raspi-car server
Project: https://github.com/LostXine/simple-raspi-car
Contact: lostxine@gmail.com
*/

#include "pi_udp.h"
#include <algorithm>

#include "rapidjson/include/rapidjson/pointer.h"

/* CRC16 Table High byte */
static unsigned char CRC16Hi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
} ;

/* CRC16 Table Low byte */
static char CRC16Lo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2,
    0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04,
    0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
    0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8,
    0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
    0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
    0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6,
    0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10,
    0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
    0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
    0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE,
    0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
    0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA,
    0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C,
    0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0,
    0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62,
    0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
    0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE,
    0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
    0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
    0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C,
    0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76,
    0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
    0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
    0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54,
    0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
    0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98,
    0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A,
    0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86,
    0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40
} ;

#define MAX_ERROR_COUNT 10

pi_driver::pi_driver(pi_serial* ps){
    LOG(INFO)<<"Driver online, id: "<<this; 
    keepRunning = false;
    err_count = 0;
    set_port(ps);
    fetching = new std::thread(fetching_thread, this);
    //wait for thread online
    while(!keepRunning){usleep(1000);}
}

pi_driver::~pi_driver(){
    keepRunning = false;
    recycle_thread(fetching);
    LOG(INFO)<<"Driver offline, id: "<<this; 
}

void pi_driver::set_port(pi_serial* ps){
    port = ps;
}

unsigned int pi_driver:: get_CRC16(unsigned char* src, int start, int fin){
    unsigned int Index ; /* will index into CRC16 lookup table */

    unsigned char CRCHi = 0xFF ; /* high byte of CRC16 initialized */
    unsigned char CRCLo = 0xFF ; /* low byte of CRC16 initialized */
    for(int i = start; i < fin; i++){
        Index = CRCHi ^ src[i] ; /* calculate the CRC16 */
        CRCHi = CRCLo ^ CRC16Hi[Index] ;
        CRCLo = CRC16Lo[Index] ;
    }
    return assemble_bytes(CRCHi, CRCLo) ;
}

void pi_driver::fill_CRC16(unsigned char* src, int start, int fin){
    unsigned int crc = get_CRC16(src, start, fin);
    src[fin] = crc >> 8;
    src[fin + 1]= crc & 0xff;
}

void pi_driver::launch_msg(unsigned char* src, int len, int tofetch){
    fetchmutex.lock();
    port->upload(src, len);
    tofetchlist.push(tofetch);
    fetchmutex.unlock();
}

void pi_driver::set_cmd(unsigned char code, int v = 0){
    int len = 4;
    unsigned char buf[] = {START_FLAG, code, END_FLAG, END_FLAG, END_FLAG, END_FLAG, END_FLAG};
    // SET
    switch(code & 0x0f){
        case 0:
            len += 2; // add 2 bytes: code mode
            buf[2] = std::max(std::min(0x01, v), 0x00);
            break;
        default:
            len += 3; // add 3 bytes: code, direction, value
            int value = std::max(std::min(100, v), -100);
            buf[3] = abs(value);
            buf[2] = (value > 0)? 0x00: 0x01;
    };
    int fetch_len = len;
    if (code >> 4){
        // GET
        len = 5;
        buf[4] = END_FLAG;
    }
    fill_CRC16(buf, 1, len - 3);
    launch_msg(buf, len, fetch_len);
}

void pi_driver::set_mode(char v){
    set_cmd(0x00, v);
}

void pi_driver::set_motor(int v){
    set_cmd(0x01, v);
}   

void pi_driver::set_servo(int v){
    set_cmd(0x02, v);
}

void pi_driver::set_cam_p(int v){
    set_cmd(0x03, v);
}

void pi_driver::set_cam_y(int v){
    set_cmd(0x04, v);
}

void pi_driver::query_mode(){
    set_cmd(0x10);
}

void pi_driver::query_motor(){
    set_cmd(0x11);
}

void pi_driver::query_servo(){
    set_cmd(0x12);
}

void pi_driver::query_cam_p(){
    set_cmd(0x13);
}

void pi_driver::query_cam_y(){
    set_cmd(0x14);
}

int pi_driver::parse_json(char* js, int len){
    rapidjson::Document d;
    d.Parse(js);
    if (d.HasParseError()) {return -1;}
    try{
        rapidjson::Value* var;
        //check mode
        if (var = rapidjson::Pointer("/so").Get(d)){
            set_mode(var->GetInt());
        }
        //check speed
        if (var = rapidjson::Pointer("/sm").Get(d)){
            set_motor(var->GetInt());
        }
        //check servo
        if (var = rapidjson::Pointer("/ss").Get(d)){
            set_servo(var->GetInt());
        }
        //check camera pitch
        if (var = rapidjson::Pointer("/scp").Get(d)){
            set_cam_p(var->GetInt());
        }
        //check camera yaw
        if (var = rapidjson::Pointer("/scy").Get(d)){
            set_cam_y(var->GetInt());
        }
        //check query requests
        if (var = rapidjson::Pointer("/rq").Get(d)){
            set_cmd(var->GetInt() & 0x0f | 0x10);
        }
    } catch(std::exception &err){
        LOG(WARNING)<<err.what();
        return 1;
    }
    return 0;
}

void pi_driver::transfer_response(unsigned char* src, int len){
    if (len < 4){
        LOG(WARNING)<<"Invalid src len";
        return;
    }
    char buf[128] = {0x00};
    switch(src[1] & 0x0f){
        case 0:
            sprintf(buf, "{\"qo\": %d}", src[2]);
            break;
        case 1:
            sprintf(buf, "{\"qm\": %d}", src[3] * ((src[2])? -1: 1));
            break;
        case 2:
            sprintf(buf, "{\"qs\": %d}", src[3] * ((src[2])? -1: 1));
            break;
    };
    if (udp){
        pi_udp* tmp = (pi_udp*)udp;
        tmp->send(buf, strlen(buf));
    }
}

void fetching_thread(pi_driver* pd){
    LOG(INFO)<<"\033[0;36mfetching thread online\033[0;0m";
    pd->keepRunning = true;
    unsigned char buf[256] = {0x00};
    while(pd->is_running()){
        if (MAX_ERROR_COUNT <= pd->err_count){
            pd->keepRunning = false;
            LOG(ERROR)<<"Driver fetching too much ("<< pd-> err_count <<") error.";
            break;
        }
        
        int tofetch = 0;
        pd->fetchmutex.lock();
        if(!pd->tofetchlist.empty()){
            tofetch = pd->tofetchlist.front();
            pd->tofetchlist.pop();
        }
        pd->fetchmutex.unlock();
        if (tofetch){
            bool toparse = tofetch < 0;
            bool tocheck = true;
            if (toparse) {tofetch = -tofetch;}
            int wait_count = 0;
            while(pd->port->fetch(buf, tofetch)){
                if(wait_count < 200){
                    usleep(10000);
                    wait_count++;
                }else{
                    pd->err_count++;
                    tocheck = false;
                    break;
                }
            }
            //check CRC
            if (tocheck){
                auto&& log = COMPACT_GOOGLE_LOG_INFO;
                log.stream()<<"\033[0;36mFetch "<< tofetch <<" bytes: ";
                for (int i = 0; i < tofetch; i++){
                    log.stream()<<std::hex<<std::setw(2)<<std::setfill('0')<<int(buf[i])<<" ";
                }
                log.stream()<<"\033[0;0m";
                if (buf[0] == START_FLAG && buf[tofetch - 1] == END_FLAG){
                    unsigned crc = pd->get_CRC16(buf, 1, tofetch - 3);
                    if (crc == pd->assemble_bytes(buf[tofetch - 3], buf[tofetch - 2])){
                        if (toparse){
                            pd->transfer_response(buf, tofetch);
                        }
                    }else{
                        LOG(WARNING)<<"recv msg CRC check failed: "
                            <<std::hex<<std::setw(4)<<std::setfill('0')<<pd->assemble_bytes(buf[tofetch - 3], buf[tofetch - 2])<<" vs. "
                            <<std::setw(4)<<std::setfill('0')<<crc;
                        pd->err_count++;
                    }
                } else {
                    LOG(WARNING)<<"recv msg HEAD/TAIL check failed: "
                    <<std::hex<<std::setw(4)<<std::setfill('0')<<pd->assemble_bytes(buf[0], buf[tofetch - 1])<<" vs. "
                    <<std::hex<<std::setw(4)<<std::setfill('0')<<pd->assemble_bytes(START_FLAG, END_FLAG);
                    pd->err_count++;
                }
            }

        }
        usleep(1000);
    }
    LOG(INFO)<<"\033[0;36mfetching thread offline\033[0;0m";
}

unsigned int pi_driver::assemble_bytes(unsigned char a, unsigned char b){
    return ((unsigned int)a << 8 | b);
}

void pi_driver::set_udp(void* _udp){
    udp = _udp;
}
