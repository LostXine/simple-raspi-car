/*
simple-raspi-car server
Project: https://github.com/LostXine/simple-raspi-car
Contact: lostxine@gmail.com
*/

#include "pi_udp.h"

#include <csignal>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

using namespace std;

static volatile bool keepRunning = true;

void sig_handler(int sig){
    if (sig == SIGINT){keepRunning = false;}
}

int main(int argc, char** argv)
{
    //parse argc
    for(int i = 1; i < argc; i++){
        if (!strcmp(argv[i],"-h")){
            printf("\033[1m\033[37mRaspiCar Server\n\r\033[0m-----\n\rBuild at %s %s\n\r", __TIME__, __DATE__);
            return 0;
        }
    }
    //check glog dir
    const char log_dir[] = "/home/pi/log/";
    if (opendir(log_dir) == NULL){mkdir(log_dir, S_IRWXU);}
    //glog init
    google::InitGoogleLogging(argv[0]);
    google::SetStderrLogging(google::GLOG_INFO);
    google::SetLogFilenameExtension("raspicar_");
    google::SetLogDestination(google::INFO, log_dir);
    FLAGS_colorlogtostderr = true;
    FLAGS_logbufsecs = 0;
    FLAGS_max_log_size = 10; //max log size: 10mb
    //glog done
    //Start main loop
    LOG(INFO)<<"\033[1m\033[37mRaspiCar Server online\033[0m"<<endl;
    LOG(INFO)<<"Build time: "<< __TIME__<<" "<<__DATE__<<endl;

    signal(SIGINT, sig_handler);

    //char pt[] = "/dev/ttyACM0";
    char pt[] = "/dev/ttyAMA0";
    pi_serial* ps = new pi_serial(pt);
    pi_driver* pd;
    pi_udp* pu;
    int port = -1;
    if (ps->get_port(port) != -1){
        try{
            pd = new pi_driver(ps);
            pu = new pi_udp(pd);
            // init config
            pd->set_motion(0.0f, 0.0f);
            pd->set_distance_num(0x00);
            //pd->set_motion(0.5f, 0.0f);
            //pd->launch_distance_mode();
            //pd->launch_speed_mode();
            LOG(INFO)<<"main loop start\033[0m";
            //int t = 0;
            while (keepRunning && pu->is_running()){
                sleep(1);
                //pd->query_status();
                //pd->query_sensor();
                //pd->set_motion(0.5 + t * 0.05, t * 0.2 - 1);
                //t++;
                //if(t > 10){t = 0;}
            }
            delete pu;
            delete pd;
            sleep(1);
            LOG(INFO)<<"main loop done\033[0m";
        }
        catch(exception &err){
            LOG(ERROR)<<err.what()<<endl;
        }
    } else {
        LOG(ERROR)<<"Can't open serail port.";
    }
    delete ps;
    LOG(INFO)<<"\033[1m\033[37mRaspiCar Server offline\033[0m"<<endl;
    google::ShutdownGoogleLogging();
return 0;
}

