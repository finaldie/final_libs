[![Build Status](https://travis-ci.org/finaldie/final_libs.svg?branch=master)](https://travis-ci.org/finaldie/final_libs)
[![GitHub license](https://img.shields.io/github/license/finaldie/final_libs.svg)]()
[![Platform](https://img.shields.io/badge/platform-Linux-blue.svg)]()
![GitHub release](https://img.shields.io/github/release/finaldie/final_libs.svg)
![GitHub closed pull requests](https://img.shields.io/github/issues-pr-closed/finaldie/final_libs.svg)
![GitHub language count](https://img.shields.io/github/languages/count/finaldie/final_libs.svg)
![GitHub code size in bytes](https://img.shields.io/github/languages/code-size/finaldie/final_libs.svg)

Common Libraries (Linux Platform)
=========================================

## Lastest Release
See [Here][1]

## ChangeLog
See [change log](ChangeLog.md)

## Library contains
Lib Name | Description |
---------|-------------|
flist    | Lockfree FIFO single-linked list in one-producer one-consumer scenario |
fdlist   | Double-linked list |
fhash    | Hash table |
flock    | A wraper, which is safer and easier to use pthread condition |
flog     | A High Performance Logging Library |
fmbuf    | A light-weight ring-buffer |
fconf    | A simple configuration file library |
ftime    | Easy to create system timer |
fthread_pool | Simple thread pool, which is easy to use |
fnet     | Wrap the system APIs, easy to use the networking api |
fev      | Event framework, including buffer, networking, timer service... |
fcache   | A simple cache with LRU |
fco      | C coroutine. **Notes:** Legacy library, use it carefully |
~~fut~~  | C Unit test framework. **Notes:** Migrated to [fcunit][4] |
~~fmempool~~ | Thread-Cache memory pool. **Notes:** Migrated to [skull-malloc][2] |
~~fpcap~~| Pcap file conversion library. **Notes:** Migrated to [fpcap][3] |

## API Documents
See [Wiki][1]

## Compile
```console
git clone git@github.com:finaldie/final_libs.git flibs
cd flibs
git submodule update --init --recursive
make
make check
make valgrind-check
```

## Benchmark
```console
make bench
make bench-run
```

## Flags
* Change the compiler, such as using clang:
```console
make CC=clang
```
* Build shared library instead of static library:
```console
make SHARED=true
```
* Build debug version without any optimization parameters
```console
make debug=true
```
* Build `32`bit libraries under `64`bit platform
```console
make BIT=32
```
* Build in parallel
```console
make -j4   # Adjust the number '4' according to the real cpu cores to speed up
the compile time
```
* Skip building legacy library
Currently, `fco` is defined as a legacy library, maybe it won't working well or
lose support in some archs, use a specify macro to skip building it.
```console
make FLIB_CFLAGS=-DFLIB_SKIP_LEGACY
```
* Custom CFLAGS and LDFLAGS
Sometimes, we want to define some different macros/compilation flags to control
compiling/linking results, in _flibs_, instead of standard `CFLAGS` and
`LDFLAGS`, we can use `FLIB_CFLAGS` and `FLIB_LDFLAGS`

## Example (on 64bit platform)
* Build 64bit static-link libraries
```console
make -j4 && make -j4 check
```
* Build 64bit dynamic-link libraries
```console
make SHARED=true -j4 && make SHARED=true check
```
* Install _flibs_ to system
After compilation, just call `install` target, to install _flibs_ into system:
```console
make install
```
**notes:** By default, the _flibs_ will be installed in **/usr/local/**:
 * Headers in: `/usr/local/include`
 * Libraries in: `/usr/local/lib`
If we want to change the location, maybe to `/usr/`, just run:
```console
make prefix=/usr/ install
```

## Use _flibs_ in a Project
After installing _flibs_ into system, basically we need few steps to use it:
 * Include the headers from the source file
 * Link the statis/dynamic library from the _Makefile_

Let's see an example _Source file_ and _Makefile_, e.g. _fhash_:
```c
// main.c
#include <stdlib.h>
#include <stdio.h>
#include <flibs/fhash.h>

int main(int argc, char** argv)
{
    // 1. Create string hash table
    fhash* tbl = fhash_str_create(0, FHASH_MASK_AUTO_REHASH);

    // 2. Set a key-value into table
    const char* key = "hello";
    fhash_str_set(tbl, key, "world");

    // 3. Get the value from table
    const char* value = fhash_str_get(tbl, key);
    printf("Key: %s, value: %s\n", key, value);

    // 4. Destroy the hash table
    fhash_str_delete(tbl);
    return 0;
}
```
```makefile
# Makefile
all:
	gcc -Wall -g -O2 -o demo main.c -lflibs
```

Then Build and Run it:
```console
final@ubuntu1404: ~/code/github/flibs/demo>make
gcc -Wall -g -O2 -o demo main.c -lflibs
final@ubuntu1404: ~/code/github/flibs/demo>./demo
Key: hello, value: world
```

Have fun :)

[1]: https://github.com/finaldie/final_libs/wiki
[2]: https://github.com/finaldie/skull-malloc
[3]: https://github.com/finaldie/fpcap
[4]: https://github.com/finaldie/fcunit
