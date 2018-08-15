# Simple Raspberry Pi Car - Arduino Nano

### Usage

* Compile and upload

1. include Chrono library in Arduino IDE (Sketch -> Include Library -> Manage Libraries..)
2. compile and upload (Only tested on Arduino Nano)

### Hardware
```
PIN 4  <- Raspberry PI: Enable</br>
PIN 9  -> Steering Servo</br>
PIN 10 -> ESC
```

### Serial Protocal
* General

For XOR:
```
0xFF databytes XOR 0xAF
|start byte    |   |end byte
               |XOR check byte from databytes
```

For CRC16(MODBUS)
```
0xFF databytes CRC16Hi CRC16Low 0xAF
|start byte    |                |end byte
               |CRC16 High and low bytes
```
Tips: All commands should begin with '0xFF' and end at '0xAF'

Calculate XOR:
```
# pseudocode
deg get_xor(databytes):
    XOR = 0x00
    for i in databytes:
        XOR ^= i
    return XOR
```

Command type:

|Code|Type|
|----|----|
|0x0.|SET|
|0x1.|GET|
|0x.0|Mode|
|0x.1|Motor|
|0x.2|Steering Servo|

* SET Mode
```
0xFF 0x00 0x00 XOR/CRC 0xAF
          |mode number
          |0x00 stop         : stop the motor
          |0x01 const voltage: give the motor constant voltage
          |0x02 ...
```

* SET Motor(Voltage/Speed)
```
0xFF 0x01 0x00 0x00 XOR/CRC 0xAF
          |mode direction
          |0x00 forward
          |else backward
               |abs value
               |0x00-0x64 -> 0.00 - 1.00
```

* SET Steering Servo
```
0xFF 0x02 0x00 0x00 XOR/CRC 0xAF
          |mode direction
          |0x00 left
          |else right
               |abs value
               |0x00-0x64 -> 0.00 - 1.00
```

* GET current setting
```
0xFF 0x1. XOR/CRC 0xAF
     |get method
     |0x10 get mode
     |0x11 get motor
     |0x12 get steering servo
```
When Arduino receives 'GET', it will response 'SET' commands above.

### Test cases(CRC16)
* All stop: 
```
FF 00 00 40 40 AF
```
* Forward 0.5:
```
FF 01 00 32 D5 A1 AF FF 00 01 40 40 AF
```
* Backward 0.3: 
```
FF 01 01 24 8B 21 AF FF 00 01 40 40 AF
```
* Get mode/motor/servo: 
```
FF 10 FF 00 AF
FF 11 FF 00 AF
FF 12 FF 00 AF
```

### Contact me
* Email: lostxine@gmail.com
