# Simple Raspberry Pi Car - Raspberry PI

### Usage

####  Compile and run raspicar-server

Depends: glog, rapidjson(included as submodule)

1. install glog
2. clone rapidjson
```
git submodule init
git submodule update
```
3. compile the server
```
cd ./server
mkdir build
cd build
cmake ..
make -j4
```
4. keep the server running
```
./raspicar_server
```

####  Run clients
* Python3:
```
cd ./client/python3
python3 ./run_raspicar.py
```

* C++:
```
cd ./client/cpp
sh ./compile.sh
./run_raspicar
```

Edit clients to complete different tasks.

### UDP Protocal

* General

UDP data is JSON in utf-8

* Client -> Server

| Key | Value | Note |
| ---- | ---- | ---- |
|sm|int: -100 ~ 100| set value related to motor|
|ss|int: -100 ~ 100| set value related to servo|
|so|int: 0~1 | set mode: 0x00 stop 0x01 const voltage|
|rq|int: 0x00 ~ 0x0f| request value |

* Server -> Client
| Key | Value | Note |
| ---- | ---- | ---- |
|qm|int: -100 ~ 100| value related to motor|
|qs|int: -100 ~ 100| value related to servo|
|qo|int: 0~1 | mode: 0x00 stop 0x01 const voltage|

### Server

* Info flow
```
Serial port <-> Driver <-> UDP -> main_loop
| recving thread  |         | listening thread
| sending thread  | fetching thread
```

* Depends
```
Serial port <- Driver <- UDP
```

### Client

### Contact me
* Email: lostxine@gmail.com
