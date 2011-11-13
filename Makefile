# Build all libs by order 
LIST = flist
HASH = fhash
LOCK = flock
MBUF = fmbuf
LOG = flog
CONF = fconf
TIMER = ftimer
MEMPOOL = fmempool
THREAD_POOL = fthread_pool
NETWORK = fnet

all:
	-( test -d $(LIST) && cd $(LIST) && make )
	-( test -d $(LIST) && cd $(LIST) && make install )
	-( test -d $(HASH) && cd $(HASH) && make )
	-( test -d $(HASH) && cd $(HASH) && make install )
	-( test -d $(LOCK) && cd $(LOCK) && make )
	-( test -d $(LOCK) && cd $(LOCK) && make install )
	-( test -d $(MBUF) && cd $(MBUF) && make )
	-( test -d $(MBUF) && cd $(MBUF) && make install )
	-( test -d $(LOG) && cd $(LOG) && make )
	-( test -d $(LOG) && cd $(LOG) && make install )
	-( test -d $(CONF) && cd $(CONF) && make )
	-( test -d $(CONF) && cd $(CONF) && make install )
	-( test -d $(TIMER) && cd $(TIMER) && make )
	-( test -d $(TIMER) && cd $(TIMER) && make install )
	-( test -d $(MEMPOOL) && cd $(MEMPOOL) && make )
	-( test -d $(MEMPOOL) && cd $(MEMPOOL) && make install )
	-( test -d $(THREAD_POOL) && cd $(THREAD_POOL) && make )
	-( test -d $(THREAD_POOL) && cd $(THREAD_POOL) && make install )
	-( test -d $(NETWORK) && cd $(NETWORK) && make )
	-( test -d $(NETWORK) && cd $(NETWORK) && make install )

.PHONY:clean
clean:
	-( test -d $(LIST) && cd $(LIST) && make clean )
	-( test -d $(HASH) && cd $(HASH) && make clean )
	-( test -d $(LOCK) && cd $(LOCK) && make clean )
	-( test -d $(MBUF) && cd $(MBUF) && make clean )
	-( test -d $(LOG) && cd $(LOG) && make clean )
	-( test -d $(CONF) && cd $(CONF) && make clean )
	-( test -d $(TIMER) && cd $(TIMER) && make clean )
	-( test -d $(MEMPOOL) && cd $(MEMPOOL) && make clean )
	-( test -d $(THREAD_POOL) && cd $(THREAD_POOL) && make clean)
	-( test -d $(NETWORK) && cd $(NETWORK) && make clean)
