# Simple Raspberry Pi Car - Raspberry PI

### Usage
* Compile and run raspicar-server

Depends: rapidjson(included) glog

```
cd ./server
mkdir build
cd build
cmake ..
make -j4
```

* Run clients

### Server

* Structure
```
Serial port <-> Driver <-> UDP
| recving thread  |        | listening thread
| sending thread  | fetching thread
```

### Client

### Contact me
* Email: lostxine@gmail.com
