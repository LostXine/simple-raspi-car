# Simple Raspberry Pi Car

A smart car project based on Raspberry PI.

***
### Update log
* 0.1
```
* Establish the project
```

***
### Hardware
Raspberry Pi <-2Pin Serial-> Arduino
Raspberry Pi <-1Pin GPIO  -> Arduino
Raspberry Pi <-2Pin Power -> Arduino
Arduino <-3Pin-> Steering Servo
Arduino <-3Pin-> ESC(Electronic Speed Controller)
ESC <-2Pin-> Brush Motor

* Power flow
```
Battery -> ESC(Electronic Speed Controller) -(BEC: 5V)-> Arduino -> Raspberry PI
                                            -(BEC: 5V)-> Steering Servo
                                            -(Battery)-> Motor
```
* Control flow
```
Raspberry PI <-Serial-> Arduino -PWM-> ESC
                                -PWM-> Steering Servo
```
### Software
```
project
    |--readme.md
    |--Arduino -> Arduino program running on Arduino Nano(ATMega328P)
    |--Raspi   -> 
        |--server -> C++11 server on Pi to communacate with Arduino
        |--client
            |--cpp    -> client demo in C++11
            |--python -> client demo in python
```

***
### Contact me
* Email: lostxine@gmail.com
