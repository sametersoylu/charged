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
After running charged (daemon), you can use dispatcher to get charge value or set critical level value to get notification as a reminder to plug your laptop to ac. 
```
chargec get charge 
chargec set critical <val>
chargec get critical
```


