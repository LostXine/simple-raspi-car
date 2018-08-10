# Simple Raspberry Pi Car

A smart car project based on Raspberry PI.


### What's up
* v0.1: Establish the project.

### Hardware
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
* Current config

|Part|Type|
|----|----|
|RC Car|Tamiya CC-01 (No.58579)|
|Raspberry PI|Raspberry PI 3B|
|Arduino|Arduino Nano (ATMega328P)|
|ESC|HobbyWing QuicRun WP 860 Dual Brushed|
|Motor|Type 540|
|Steering Servo|Futaba S3003|

### Software
```
project
    |--readme.md
    |--Arduino -> Arduino program
    |--Raspi   -> PI program
        |--server -> C++11 server on Pi to communacate with Arduino
        |--client
            |--cpp    -> client demo in C++11
            |--python -> client demo in python
```

### Contact me
* Email: lostxine@gmail.com
