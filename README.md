[![Build Status](https://travis-ci.org/finaldie/final_libs.svg?branch=0.9)](https://travis-ci.org/finaldie/final_libs)

Common Libraries (Linux Platform)
=========================================

## Lastest Release
See [Here][1]

## ChangeLog
See [change log](ChangeLog.md)

## Library contains
Lib Name | Description |
---------|-------------|
flist    | Lockfree list in one-producer one-consumer |
fhash    | Hash table |
flock    | A wraper, which safe and easy to use pthread condition |
flog     | A High Performance Logging System |
fmbuf    | A light-weight buffer |
fconf    | A simple format config lib |
ftime    | Easy to create system timer |
fthread_pool | Simple thread pool, which easy to use |
fnet     | Wrap the system api, easy to use the network api |
fev      | Event framework, including buffer, network, timer service |
fcache   | A simple cache with LRU |
fco      | C coroutine |
~~fut~~  | Unit test framework. **notes:** Migrated to [fcunit][4] |
~~fmempool~~ | Thread cache memory pool. **notes:** Migrated to [skull-malloc][2] |
~~fpcap~~| Pcap file convertion lib. **notes:** Migrated to [fpcap][3] |

**NOTE:** After run as above, you can find the libraries have been installed in the specified folder or current folder, which named as "final_libraries"

## API Documents
See [Wiki][1]

## Compile
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
make bench
make bench-run
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
make debug=true
```
* Build `32`bit libraries under `64`bit platform
```
make BIT=32
```
* Build in parallel
```
make -j4   # replace the number 4 according to your real cpu number to speed up the compile time
```

## Example (on 64bit platform)
* Build 64bit static-link libraries
```
make -j4 && make -j4 check
```
* Build 64bit dynamic-link libraries
```
make SHARED=true -j4 && make SHARED=true check
```
* Install flibs to system
```
make install
```
**notes:** By default, the `prefix` is **/usr/local**, so the flibs will be installed to **/usr/local**, if you want to change the prefix, just run the following:
```
make SHARED=true prefix=$(other_location) -j4
make prefix=$(other_location) install
```

## Use _flibs_ in a Project
After installing _flibs_ into system, basically we need few steps to use it:
 * Include the headers from your source file
 * Link the statis/dynamic library from your _Makefile_

Let's see an example _Source file_ and _Makefile_, e.g. use _fhash_:
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
