# Simple Raspberry Pi Car - Arduino

### Hardware
PIN 4  <- Raspberry PI: Enable</br>
PIN 9  -> Steering Servo</br>
PIN 10 -> ESC

### Protocal
* General
```
0xFF databytes XOR 0xAF
|start byte        |end byte
                |XOR check byte from databytes
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
0xFF 0x00 0x00 XOR 0xAF
          |mode number
          |0x00 stop         : stop the motor
          |0x01 const voltage: give the motor constant voltage
          |0x02 ...
```

* SET Motor(Voltage/Speed)
```
0xFF 0x01 0x00 0x00 XOR 0xAF
          |mode direction
          |0x00 forward
          |else backward
               |abs value
               |0x00-0x64 -> 0.00 - 1.00
```

* SET Steering Servo
```
0xFF 0x02 0x00 0x00 XOR 0xAF
          |mode direction
          |0x00 left
          |else right
               |abs value
               |0x00-0x64 -> 0.00 - 1.00
```

* GET current setting
```
0xFF 0x1. XOR 0xAF
     |get method
     |0x10 get mode
     |0x11 get motor
     |0x12 get steering servo
```
When Arduino receives 'GET', it will response 'SET' commands above.

### Test cases
* All stop: FF 00 00 00 AF
* Forward 0.5: FF 01 00 32 33 AF FF 00 01 01 AF
* Backward 0.3: FF 01 01 24 24 AF FF 00 01 01 AF
* Get mode: FF 10 10 AF


### Contact me
* Email: lostxine@gmail.com
