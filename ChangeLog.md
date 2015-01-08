* 2015-01-10 0.7.1
   * Remove `fmempool`, which has already been migrate to `skull-malloc` repo
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
