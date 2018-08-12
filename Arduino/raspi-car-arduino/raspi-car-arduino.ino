/*
simple-raspi-car: Arduino Nano
lostxine@gmail.com
Aug.10, 2018
Compile: 
1.include Chrono library in Arduino IDE (Sketch -> Include Library -> Manage Libraries..)
2.compile and upload
*/

//#define PRINT_ASCII       //reply ACSII chars when defined

/* --- CONFIG ---*/
#define BAUD_RATE  115200 // baud rate
#define RPI_ENABLE 4      // raspi: enable
#define ESC_SIGN   10     // esc pin
#define STR_SIGN   9      // steering pin
#define START_FLAG 0xFF   // start flag
#define END_FLAG   0xAF   // end flag
#define BUF_LEN    16     // buffer length
#define MODE_MAX   0x01   // max possible mode
#define VALUE_MAX  0x64   // max possible value

// control paras
#define CONTROL_T 50      // control interval(ms) because PWM interval is 16-20ms
#define WATCH_DOG 100     // watchdog times
//Watchdog interval= WATCH_DOG * CONTROL_T = 5000ms = 5s

// motor limits
#define MOTOR_MID 90      // motor voltage mid
#define MOTOR_FWD 65      // motor voltage min
#define MOTOR_BCK 105     // motor voltage max

// steering servo limits
#define SERVO_MID 90      // steering servo mid
#define SERVO_RGT 110     // steering servo right limit
#define SERVO_LFT 70      // steering servo left limit

/* --- No need to change ---*/
// include headers
#include <Servo.h>       // servo driver
#include <Chrono.h>       // timer

// global objects
Servo m_servo;           // steering servo object
Servo m_motor;           // motor object
Chrono c_timer;           // control timmer

// global vars
char c_set_motor = 0;  // motor voltage
char c_set_servo = 0;  // steering servo value
byte  b_set_mode = 0;     // mode 
byte  bl_r_buff[BUF_LEN] = {0};// read buffer
byte  bl_w_buff[BUF_LEN] = {0};// write buffer
int   i_watch_dog = 0;    // watch dog adder
int   i_s_idx = 0;        // i_serial
bool  o_s_reading = false;// isReadingCommand

/* --- defined functions ---*/
// set motor volt
void set_motor_volt(char b) {
  float s = b / 100.0f;
  float value = MOTOR_MID;
  if (s > 0) {
    value += s * (MOTOR_FWD - MOTOR_MID);
  } else {
    value += s * (MOTOR_MID - MOTOR_BCK);
  }
  m_motor.write(value);
}

// set servo
// -0x64 - 0.0 - 0x64
//  left  mid  right
void set_servo(char b) {
  float s = b / 100.0f;
  float value = SERVO_MID;
  if (s > 0) {
    value += s * (SERVO_MID - SERVO_RGT);
  } else {
    value += s * (SERVO_LFT - SERVO_MID);
  }
  m_servo.write(value);
}

// set mode
void set_mode(byte m){
  b_set_mode = min(max(m, 0), 0x02);  
}

void emergency_stop(){
  set_mode(0);
  set_motor_volt(0.0);
}

void reset_all(){
  set_mode(0);
  set_motor_volt(0.0);
  set_servo(0.0);
}

// check raspi enable GPIO
bool check_enable(){
  bool enable = analogRead(RPI_ENABLE) > 600;
  digitalWrite(LED_BUILTIN, enable);
  return enable;
}

// calculate and fill XOR
byte get_xor(byte* buff, int start, int fin){
  byte xr = buff[start];
  for(int i = start + 1; i < fin; i++){
    xr ^= buff[i];
  }
  return xr;
}

void set_xor(int fin){
  bl_w_buff[fin] = get_xor(bl_w_buff, 1, fin);
}

void clean_buff(byte* buff){
  memset(buff, 0x00, BUF_LEN); // clean cache
}

void send_buff(int end_idx){
  byte* t = bl_w_buff;
  t[0] = START_FLAG;
  set_xor(end_idx);
  t[end_idx + 1] = END_FLAG;
  Serial.write(t, end_idx + 2);
  clean_buff(t); // clean after writing
}

byte limit_range(byte v, byte limit){
  return max(min(v, limit), 0);
}

char parse_two_bytes(byte sig, byte val){
  char v = limit_range(val, VALUE_MAX);
  if (sig){v = -v;}
  return v;
}

// parse read commands
void parse_read(int len){
  byte* t = bl_r_buff;
  byte* f = bl_w_buff;
  // XOR check
  if (get_xor(t, 0, len - 1) == t[len - 1]){
    i_watch_dog = 0; // feed the dog
    int end_idx = 2; // Start FLAG + XOR byte
    switch(t[0] & 0x0f){
      case 0x00: // mode
          end_idx += 1; //mode take 2 bytes
          if (t[0] < 0x10){b_set_mode = limit_range(t[1], MODE_MAX);} // set
          f[2] = b_set_mode;
        break;
      case 0x01: // motor
          end_idx += 2; // motor and servo take 3 bytes
          if (t[0] < 0x10){c_set_motor = parse_two_bytes(t[1], t[2]);} // set
          f[2] = c_set_motor > 0;
          f[3] = abs(c_set_motor);
        break;
      case 0x02: // servo
          end_idx += 2;
          if (t[0] < 0x10){c_set_servo = parse_two_bytes(t[1], t[2]);} // set
          f[2] = c_set_servo > 0;
          f[3] = abs(c_set_servo);
        break;
    }
    f[1] = 0x0f & t[0]; // fill get->set
    send_buff(end_idx);
  } else {
    #ifdef PRINT_ASCII
    Serial.println("XOR failed.");
    #endif
  }
  
}

/* --- default functions ---*/
void setup() {
  // setup serail port
  Serial.begin(BAUD_RATE);
  while(Serial.read()>= 0){} //clear serialbuffer
  Serial.setTimeout(5);
  // serial setup done
  #ifdef PRINT_ASCII
  Serial.println("Simple-raspi-car: Arduino setup start");
  #endif
  // setup pins
  m_motor.attach(ESC_SIGN);     // attach esc
  m_servo.attach(STR_SIGN);     // attach steering servo
  pinMode(RPI_ENABLE, INPUT);   // attach raspi GPIO
  pinMode(LED_BUILTIN, OUTPUT); // setup LED
  // reset all paras
  reset_all();
  // start done
  #ifdef PRINT_ASCII
  Serial.println("Simple-raspi-car: Arduino setup done");
  #endif
}

void loop() {
  // watch_dog
  if (i_watch_dog > WATCH_DOG){
    // trig watch_dog
    i_watch_dog = 0;
    set_mode(0);
    #ifdef PRINT_ASCII
    Serial.println("Watch dog!");
    #endif
  }
  // timer triggered
  if (c_timer.hasPassed(CONTROL_T)){
    // reset timer
    c_timer.restart();
    i_watch_dog += 1;
    if (check_enable()){
      // enable
      if (b_set_mode == 0x00){emergency_stop();} // stop mode
      else{set_servo(c_set_servo);}              // otherwise set servo
      switch(b_set_mode){
        case 0x01:
          // const volt mode
          set_motor_volt(c_set_motor);
          break;
      }
    } else {
        reset_all();
        #ifdef PRINT_ASCII
        Serial.println("Raspi offline!");
        #endif
      }
  }
}

// Serial
void serialEvent(){
  byte key = Serial.read();
  if (o_s_reading){
    // waiting for ending
    if (key == END_FLAG){
      o_s_reading = false;
      parse_read(i_s_idx);
    } else {
      // record the commands
      bl_r_buff[i_s_idx] = key;
      i_s_idx++;
      if (i_s_idx == BUF_LEN){
          #ifdef PRINT_ASCII
          Serial.println("Out of recv buff.");
          #endif
          o_s_reading = false;
      }
    }
  } else{
    // waiting for command start
    if (key == START_FLAG){
      // ready to start
      o_s_reading = true;
      i_s_idx = 0;
      clean_buff(bl_r_buff);
    }
  }
}

