/*
simple-raspi-car: Arduino Nano
lostxine@gmail.com
Aug.10, 2018
Compile: 
1.include Chrono library in Arduino IDE (Sketch -> Include Library -> Manage Libraries..)
2.compile and upload
*/

//#define PRINT_ASCII       //reply ACSII chars when defined
#define CRC16            // using CRC16(Modbus) rather than XOR

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
bool  o_f_raspi = false;  // raspi flag

// CRC 16
/* CRC16 Table High byte */
byte CRC16Hi[] = {
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
byte CRC16Lo[] = {
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
  bool enable = digitalRead(RPI_ENABLE);
  digitalWrite(LED_BUILTIN, enable);
  return enable;
}

#ifdef CRC16
unsigned int assemble_bytes(byte hi, byte low){
  return ((unsigned int)hi << 8 | low);
}

// calculate and fill CRC16
unsigned int get_crc16(byte* buff, int start, int fin){
  byte CRCHi = 0xff;
  byte CRCLo = 0xff;
  unsigned int index;
  for(int i = start; i < fin; i++){
    index = CRCHi ^ buff[i];
    CRCHi = CRCLo ^ CRC16Hi[index];
    CRCLo = CRC16Lo[index];
  }
  return assemble_bytes(CRCHi, CRCLo);
}

void set_crc16(int fin){
  unsigned int crc = get_crc16(bl_w_buff, 1, fin);
  bl_w_buff[fin] = byte(crc >> 8);
  bl_w_buff[fin + 1] = byte(crc & 0xff);
}
#else
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
#endif
void clean_buff(byte* buff){
  memset(buff, 0x00, BUF_LEN); // clean cache
}

void send_buff(int end_idx){
  byte* t = bl_w_buff;
  t[0] = START_FLAG;
  #ifdef CRC16
  set_crc16(end_idx);
  end_idx++;
  #else
  set_xor(end_idx);
  #endif
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
  if (
    #ifdef CRC16
    get_crc16(t, 0, len - 2) == assemble_bytes(t[len - 2], t[len - 1])
    #else
    get_xor(t, 0, len - 1) == t[len - 1]
    #endif
    ){
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
    #ifdef CRC16
    Serial.println("CRC16 failed.");
    #else
    Serial.println("XOR failed.");
    #endif
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
      if (!o_f_raspi){
          o_f_raspi = true;
          #ifdef PRINT_ASCII
          Serial.println("Raspi online!");
          #endif
      }
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
        if (o_f_raspi){
          o_f_raspi = false;
          #ifdef PRINT_ASCII
          Serial.println("Raspi offline!");
          #endif
        }
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

