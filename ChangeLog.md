* 2017-07-24 0.9.9
   * Remove all the `__WORDSIZE` macro detection
   * Support compiling on `musl` libc
   * Support `FLIB_CFLAGS` and `FLIB_LDFLAGS` user defined macros
   * Cleanup useless files
   * Refine README
* 2017-04-10 0.9.8
   * **flog:** Log the content sequentially
   * **fmbuf:** Add fmbuf_empty()
* 2017-01-09 0.9.7
   * **fnet:** Fix sockaddr issues in fnet_(peer|sock)name for AF_INET6
* 2016-12-17 0.9.6
   * **fev:** Add more interfaces to fev_timer_service, user would manually trigger all expired timers
* 2016-12-15 0.9.5
   * **fnet:** Support IPv6
   * **fev:** Add prepoll/postpoll for fev, and prevent output errors when del event on a NIL masked fd
* 2016-10-30 0.9.4
   * **fhash:** Data address must be 32 bit-aligned
* 2016-10-29 0.9.3
   * Fix padding issue in 32bit mode
   * Compatible with Raspberry PI
* 2016-10-27 0.9.2
   * **MISC:** Provide all-in-one library `libflibs.(a|so)` for users
   * **MISC:** Refine README
* 2016-10-14 0.9.1
   * **flist:** Add memory barrier to prevent complier/runtime reordering
   * **fmbuf:** Add memory barrier to prevent complier/runtime reordering
* 2016-08-12 0.8.13
   * **fnet:** Enhance fnet_accept
   * **fev:** Add fev_tmsvc_resetn api and fix calling getrlimit with wrong type
   * **flog:** Increase default file name max length
* 2016-07-19 0.8.12
   * **fev:** Fix the write event cannot be triggered in some OS releases
   * **fhash:** Fix compiling issue on lower version of gcc
   * **fcache:** Fix compiling issue on lower version of gcc
   * **fnet:** Fix compiling issue on lower version of gcc
* 2016-01-18 0.8.11
   * **fdlist:** Fix fdlist_foreach bug
* 2016-01-15 0.8.10
   * **fdlist:** fdlist_foreach support user arg
* 2016-01-03 0.8.9
   * **fhash:** Make fhash set api more user friendly
* 2015-12-30 0.8.8
   * **flog:** Use low-level write instead of fwrite, to fix log content lost during the shutdown phase
* 2015-12-30 0.8.7
   * **flog:** Make it fully graceful shutdown and fix the pthread valgrind warning
   * **fnet:** Correct read/write errno checking
   * **fev_buff:** Correct read/write errno checking
* 2015-12-28 0.8.6
   * **fhash:** Fix hash_node_set_key() memcpy overlap issue
* 2015-12-26 0.8.5
   * **flist:** Correct the parameter type
   * Fix valgrind errors on ubuntu 14.04
* 2015-12-21 0.8.4
   * **Fix:** `fev` -> when EPOLLHUP bit be filled, we should still trigger read callback
* 2015-10-14 0.8.3
   * Fix `flog` potential coredump when the process exit
* 2015-09-25 0.8.2
   * Remove `fmbuf_create1` api, `fmbuf_create` use `calloc` instead of `malloc`
* 2015-02-12 0.8.1
   * Remove fut, use fcunit instead
   * Change API name `fgettime` to `ftime_gettime`
* 2015-01-21 0.7.5
   * **Fix:** Remove the useless #pragma pack()
* 2015-01-13 0.7.4
   * Refactor all the makefiles, now it can be built in parallel
   * Refactor the header files folder structure, to make them more user friendly
   * Fefactor some apis, to make them more user friendly
   * Fix the valgrind errors when building the libs in 32 bit platform
* 2015-01-10 0.7.3
   * **flog:** Remove the useless print
* 2015-01-10 0.7.2
   * Remove `fpcap`, which has already been migrated to `fpcap` repo
* 2015-01-10 0.7.1
   * Remove `fmempool`, which has already been migrated to `skull-malloc` repo
* 2014-12-24 0.6.6
   * **fhash:** fhash_[int/u64/str] should not accept the NULL parameter
* 2014-11-07 0.6.5
   * **flog:** Add millisecond in timestamp
   * **flog:** Remove useless padding from internel protocol
* 2014-11-06 0.6.4
   * **flog:** Use the fixed log name, only change the name for the rolling files
* 2014-10-23 0.6.3
   * **flog:** Add flog_vset_cookie api
* 2014-10-23 0.6.2
   * **flog:** Refine log file format
   * **flog:** Add flog_vwritf api
   * **flog:** Add more metrics for log
* 2014-10-20 0.6.1
   * Add flog_set_cookie api
   * Add fsync force flog flush kernel buffer to disk
   * Fix fmbuf segfault when fmbuf_pop a ring buffer while the buf arg is NULL
* 2014-10-16 0.6.0
   * Refine flog code structure and api, split original headers to 3 new headers
* 2014-10-15 0.5.3
   * Add -fstack-protector flag
* 2014-08-27 0.5.2
   * Remove the lib64 folder, both 32 and 64bit library will be installed in the lib folder
* 2014-07-21 0.5.1
   * Refactor `fmbuf`, make the api more user friendly
* 2014-05-29 0.4.10
   * Add `make doc` to generate api wiki
* 2014-05-24 0.4.9
   * Refactor fhash
* 2014-05-04 0.4.8
   * Refactor Makefile
   * exit non-zero when UT failure
* 2014-03-09 0.4.7
   * Fix the clock resolution higher than 1ms in some systems
* 2013-09-04 0.4.6
   * Fix the api name for fread_conf library
* 2013-09-04 0.4.5
   * Refine Makefile and README
* 2013-08-26 0.4.4
   * Fully support valgrind-check
   * Add timer service and integrate it into async_conn
* 2013-07-25 0.4.3
   * Add a sub folder for headers in include folder, which used for easy to maintain the headers
* 2013-07-25 0.4.2
   * Refine the return code of fpcap_convert
* 2013-07-25 0.4.1
   * Refine flist, remove the pl_mgr declare, instead of using flist*
   * Add a new api for flist -- flist_tail()
* 2013-07-24 0.4.0
   * Refine all filenames and api names
   * Refine all Makefiles
   * Support both 32 & 64 bit libs
* 2013-07-10 0.3.8
   * Refine flist's code
   * Fix coredump when open pcap file failed
* 2013-06-27 0.3.7
   * Add flist_sort api
   * Enhance the pcap lib, which expose tcp syn & ack flags
* 2013-03-28 0.3.6
   * Add pcap convertion feature
* 2013-03-12 0.3.5
   * Add hash set/get/del for uint64 api
* 2012-12-06 0.3.3
   * Issue #11 fix dead loop when no space on disk
* 2012-11-28 0.3.2
   * Add new interface -- fev_add_listener_byfd
