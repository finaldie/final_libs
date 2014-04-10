[![Build Status](https://travis-ci.org/finaldie/final_libs.svg?branch=0.4)](https://travis-ci.org/finaldie/final_libs)

Final's common libraries (linux platform)
Email: hyzwowtools@gmail.com

[Lastest Versions]:

+--------+--------------------+------------+-----------------------------------------+
| Branch | Lastest Stable Tag |  Released  | Description                             |
+--------+--------------------+------------+-----------------------------------------+
|  0.4   | 0.4.7              | 2014-03-09 | Full 32 & 64 bit support                |
+--------+--------------------+------------+-----------------------------------------+
|  0.3   | 0.3.8              | 2013-07-10 | Full libs support, 64bit                |
+--------+--------------------+------------+-----------------------------------------+

[Describe]:

Library contains:
    1.  flist         -- Lockfree list in one producer one consumer
    2.  fhash         -- Hash table
    3.  flock         -- Easy to use condition
    4.  flog          -- A High Performance Log System: Thread-Caching Log System
    5.  fmbuf         -- A light-weight buffer
    6.  fconf         -- A simple format config system
    7.  ftimer        -- Easy to create system timer
    8.  fmempool      -- Thread cache memory pool
    9.  fthread_pool  -- Simple thread pool, which easy to use
    10. fnet          -- Wrap the system api, easy to use the network api
    11. fev           -- Event framework, including buffer, network, timer service
    12. ftu           -- Unit test framework
    13. fcache        -- A simple cache with LRU
    14. fco           -- C coroutine support
    15. fpcap         -- Pcap file convertion lib

[INSTALL Fat]:
    1. make or make prefix=/usr/
    2. make check
    3. make valgrind-check

    note: After run as above, you can find the libraries have been installed in the specified folder or current folder, which named as "final_libraries"

[INSTALL 32bit only]
    1. make all32 or make prefix=/usr/ all32
    2. make check32
    3. make valgrind-check32

[INSTALL 64bit only]
    1. make all64 or make prefix=/usr/ all64
    2. make check64
    3. make valgrind-check64

[Notes]
    1. if you want to change the compiler, such as using clang: make CC=clang
    2. if you want to use shared library instead of static library: make SHARED=true

[ChangeLog]
2014-03-09 0.4.7
   * Fix the clock resolution higher than 1ms in some systems
2013-09-04 0.4.6
   * Fix the api name for fread_conf library
2013-09-04 0.4.5
   * Refine Makefile and README
2013-08-26 0.4.4
   * Fully support valgrind-check
   * Add timer service and integrate it into async_conn
2013-07-25 0.4.3
   * Add a sub folder for headers in include folder, which used for easy to maintain the headers
2013-07-25 0.4.2
   * Refine the return code of fpcap_convert
2013-07-25 0.4.1
   * Refine flist, remove the pl_mgr declare, instead of using flist*
   * Add a new api for flist -- flist_tail()
2013-07-24 0.4.0
   * Refine all filenames and api names
   * Refine all Makefiles
   * Support both 32 & 64 bit libs
2013-07-10 0.3.8
   * Refine flist's code
   * Fix coredump when open pcap file failed
2013-06-27 0.3.7
   * Add flist_sort api
   * Enhance the pcap lib, which expose tcp syn & ack flags
2013-03-28 0.3.6
   * Add pcap convertion feature
2013-03-12 0.3.5
   * Add hash set/get/del for uint64 api
2012-12-06 0.3.3
   * Issue 11: fix dead loop when no space on disk
2012-11-28 0.3.2
   * Add new interface -- fev_add_listener_byfd
