[![Build Status](https://travis-ci.org/finaldie/final_libs.svg?branch=0.7)](https://travis-ci.org/finaldie/final_libs)

Common Libraries (Linux Platform)
=========================================

## Lastest Release
See [Here][1]

## Library contains
Lib Name | Description |
---------|-------------|
flist    | Lockfree list in one producer one consumer |
fhash    | Hash table |
flock    | Easy to use condition |
flog     | A High Performance Log System: Thread-Caching Log System |
fmbuf    | A light-weight buffer |
fconf    | A simple format config lib |
ftimer   | Easy to create system timer |
fmempool | Thread cache memory pool |
fthread_pool | Simple thread pool, which easy to use |
fnet     | Wrap the system api, easy to use the network api |
fev      | Event framework, including buffer, network, timer service |
ftu      | Unit test framework |
fcache   | A simple cache with LRU |
fco      | C coroutine support |
fpcap    | Pcap file convertion lib |

**NOTE:** After run as above, you can find the libraries have been installed in the specified folder or current folder, which named as "final_libraries"

## API Documents
See [Wiki][1]

## Compile and INSTALL
```c
git clone git@github.com:finaldie/final_libs.git flibs
cd flibs
git submodule update --init --recursive
make
make check
make valgrind-check
```

## Benchmark
```
make benchmark
make benchmark-run
```

## Flags
* Change the compiler, such as using clang:
```
make CC=clang
```
* Build shared library instead of static library:
```
make SHARED=true
```
* Build debug version without any optimization parameters
```
make MODE=debug
```
* Show verbose output
```
make VERBOSE=true
```
* Build `32`bit libraries under `64`bit platform
```
make BIT=32
```

## ChangeLog
See [change log](ChangeLog.md)

[1]: https://github.com/finaldie/final_libs/wiki
