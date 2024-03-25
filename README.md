# charged
A simple critical charge level reminder\
Works only on linux

## Build
Build requirements
```
cmake gcc
```

Build
```
mkdir build 
cd build 
cmake -S ../ -B ./
cmake --build .
```
After building you should copy the executable to 

## Usage 
```
charger -cl 20 -ll 25 # cl stands for critical level, ll stands for low level
```
After running charger (daemon), you can use dispatcher to get charge value or set critical level value to get notification as a reminder to plug your laptop to ac.
```
charged get charge level
charged set critical level <val>
charged get critical level
charged set low level <val>
charged get low level
```


