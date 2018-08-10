# Simple Raspberry Pi Car - Arduino

### Hardware
PIN 4  <- Raspberry PI: Enable
PIN 9  -> Steering Servo
PIN 10 -> ESC

### Protocal
* General
```
0x15 databytes XOR 0x51
|start byte    |   |end byte
               |XOR check byte from databytes
```
Tips: All commands should begin with '0x15' and end at '0x51'

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
|0xF.|GET|
|0x.0|Mode|
|0x.1|Motor|
|0x.2|Steering Servo|

* SET Mode
```
0x15 0x00 0x00 XOR 0x51
          |mode number
          |0x00 stop         : stop the motor
          |0x01 const voltage: give the motor constant voltage
          |0x02 ...
```

* SET Motor(Voltage/Speed)
```
0x15 0x01 0x00 0x00 XOR 0x51
          |mode direction
          |0x00 forward
          |else backward
               |abs value
               |0x00-0x64 -> 0.00 - 1.00
```

* SET Steering Servo
```
0x15 0x02 0x00 0x00 XOR 0x51
          |mode direction
          |0x00 left
          |else right
               |abs value
               |0x00-0x64 -> 0.00 - 1.00
```

* GET current setting
```
0x15 0xF. 0x51
     |get method
     |0xF0 get mode
     |0xF1 get motor
     |0xF2 get steering servo
```
When Arduino receives 'GET', it will response 'SET' commands above.

### Contact me
* Email: lostxine@gmail.com
