#include "driver.h"
#include <unistd.h>
#include <csignal>
#include <cmath>

using namespace std;

bool kp1,kp2;

void sig_handler(int sig){
    if(sig == SIGINT){
        kp1 = !kp1;
        kp2 = !kp2;
    }
}

int main(){
    kp1 = true;
    kp2 = false;
    signal(SIGINT, sig_handler);
    driver* d = new driver();
    d->setMotion(0.0f, 0.0f);
    d->allStop();
    int t = 10;
    bool b = true;
    printf("Test 1/2: voltage mode\n");
    d->launchVoltageMode();
    while(kp1){
        float st = t * 0.1 - 1;
        float sm = st;
        if (abs(st) < 0.4) {
            sm = (st > 0)? 0.4: -0.4;
            if (abs(st) < 0.01){
                sm = 0;
            }
        }
        d->setMotion(sm, st);
        printf("Motor: %0.2f, Servo: %0.2f\n", sm, st);
        if (b){t++;}else{t--;}
        if (t > 20) {b = false; t-=2;}
        if (t < 0)  {b = true; t+=2;}
        sleep(1);
        d->getMotor();
        d->getServo();
        d->getMode();
        sleep(1);
    };
    while(kp2){
        st = t * 0.1 - 1;
        sm = st;
        d->setMotion(sm, st);
        printf("Motor: %0.2f, Servo: %0.2f\n", sm, st);
        if (b){t++;}else{t--;}
        if (t > 20) {b = false; t-=2;}
        if (t < 0)  {b = true; t+=2;}
        sleep(1);
        d->getMotor();
        d->getServo();
        d->getMode();
        sleep(1);
    };
    d->setMotion(0.0, 0.0);
    d->setDistance(0x00);
    d->allStop();
    delete d;
    return 0;
}
