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
python3 ./raspi_car_demo.py
```

* C++:
```
cd ./client/cpp
sh ./compile.sh
./run_picar
```

### Server

* Structure
```
Serial port <-> Driver <-> UDP -> main_loop
| recving thread  |         | listening thread
| sending thread  | fetching thread
```

### Client

### Contact me
* Email: lostxine@gmail.com
