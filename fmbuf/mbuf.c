#include <stdlib.h>
#include <string.h>
#include "mbuf.h"

#define MBUF_START(pbuf)			( pbuf->buf )
#define MBUF_END(pbuf)				( pbuf->buf + pbuf->size - 1 )
#define	MBUF_HEAD(pbuf) 			( pbuf->head )
#define	MBUF_TAIL(pbuf) 			( pbuf->tail )
#define MBUF_SIZE(pbuf)				( pbuf->size )
#define MBUF_USED(pbuf)				( MBUF_HEAD(pbuf) <= MBUF_TAIL(pbuf) ? MBUF_TAIL(pbuf) - MBUF_HEAD(pbuf) : MBUF_SIZE(pbuf) + MBUF_TAIL(pbuf) - MBUF_HEAD(pbuf) )
#define MBUF_FREE(pbuf)				( MBUF_HEAD(pbuf) <= MBUF_TAIL(pbuf) ? MBUF_SIZE(pbuf) + MBUF_HEAD(pbuf) - MBUF_TAIL(pbuf) : MBUF_HEAD(pbuf) - MBUF_TAIL(pbuf) )
#define	MBUF_FIX_ADDR(size)			( size + ((size & 3) > 0)*(4 - (size & 3))  )		//fix the addr 

typedef unsigned int 	uint;

struct _mbuf
{
	uint	size;
	char*	head;
	char*	tail;
	char	buf[1];
};

mbuf*	create_mbuf(size_t size)
{
	if( size > 0 )
	{
		mbuf* pmbuf = (mbuf*)malloc(sizeof(mbuf) + size);

		if( !pmbuf ) return NULL;
		pmbuf->size = size;
		pmbuf->head = pmbuf->tail = pmbuf->buf;

		return pmbuf;
	}
	return NULL;
}

void	delete_mbuf(mbuf* pbuf)
{
	free(pbuf);
}

int		mbuf_push( mbuf* pbuf, const void* data, size_t size)
{
	if( pbuf && data && size > 0 && (uint)MBUF_FREE(pbuf) >= size )
	{
		uint tail_free = MBUF_END(pbuf) - MBUF_TAIL(pbuf) + 1;
				
		if( tail_free >= size )
		{	
			memcpy(MBUF_TAIL(pbuf), (char*)data, size);
			MBUF_TAIL(pbuf) += size;

			if( MBUF_TAIL(pbuf) > MBUF_END(pbuf) )
				MBUF_TAIL(pbuf) = MBUF_START(pbuf);
		}
		else
		{
			memcpy(MBUF_TAIL(pbuf), (char*)data, tail_free);
			uint left = size - tail_free;
			memcpy(MBUF_START(pbuf), (char*)data+tail_free, left);
			MBUF_TAIL(pbuf) = MBUF_START(pbuf) + left;
		}
		
		return 0;	//push sucess
	}
	
	return 1;		//push failed
}


//pop data from head
//if head to end space is less than size then go on to pop from start
int		mbuf_pop( mbuf* pbuf, void* data, size_t size)
{
	if( pbuf && data && size > 0 && size <= (uint)MBUF_USED(pbuf) )
	{
		uint tail_use = MBUF_END(pbuf) - MBUF_HEAD(pbuf) + 1;

		if( tail_use >= size ) 
		{
			memcpy((char*)data, MBUF_HEAD(pbuf), size);
			MBUF_HEAD(pbuf) += size;

			if( MBUF_HEAD(pbuf) > MBUF_END(pbuf) )
				MBUF_HEAD(pbuf) = MBUF_START(pbuf);
		}
		else
		{
			memcpy((char*)data, MBUF_HEAD(pbuf), tail_use);
			uint left = size - tail_use;
			memcpy((char*)data+tail_use, MBUF_START(pbuf), left);
			MBUF_HEAD(pbuf) = MBUF_START(pbuf) + left;
		}

		return 0;	//pop sucess
	}

	return	1;		//pop failed
}

void*	mbuf_vpop(mbuf* pbuf, void* data, size_t size)
{
	if( pbuf && data && size > 0 && size <= (uint)MBUF_USED(pbuf) )
	{
		uint tail_use = MBUF_END(pbuf) - MBUF_HEAD(pbuf) + 1;

		if( tail_use >= size ) 
		{
			data = MBUF_HEAD(pbuf);
			MBUF_HEAD(pbuf) += size;

			if( MBUF_HEAD(pbuf) > MBUF_END(pbuf) )
				MBUF_HEAD(pbuf) = MBUF_START(pbuf);
		}
		else
		{
			memcpy((char*)data, MBUF_HEAD(pbuf), tail_use);
			uint left = size - tail_use;
			memcpy((char*)data+tail_use, MBUF_START(pbuf), left);
			MBUF_HEAD(pbuf) = MBUF_START(pbuf) + left;
		}

		return data;	//pop sucess
	}

	return	NULL;		//pop failed
}

// only use to move the tail ptr for sometime need performance
void	mbuf_tail_move(mbuf* pbuf, size_t size)
{
	if( pbuf && size > 0 && size <= (uint)MBUF_USED(pbuf) )
	{
		uint tail_use = MBUF_END(pbuf) - MBUF_HEAD(pbuf) + 1;

		if( tail_use >= size ) 
		{
			MBUF_HEAD(pbuf) += size;

			if( MBUF_HEAD(pbuf) > MBUF_END(pbuf) )
				MBUF_HEAD(pbuf) = MBUF_START(pbuf);
		}
		else
		{
			uint left = size - tail_use;
			MBUF_HEAD(pbuf) = MBUF_START(pbuf) + left;
		}
	}
}

void* 	mbuf_getraw(mbuf* pbuf, void* data, size_t size)
{
	if( pbuf && data && size > 0 && size <= (uint)MBUF_USED(pbuf) )
	{
		uint tail_use = MBUF_END(pbuf) - MBUF_HEAD(pbuf) + 1;

		if( tail_use >= size ) 
		{
			data = MBUF_HEAD(pbuf);
		}
		else
		{
			memcpy((char*)data, MBUF_HEAD(pbuf), tail_use);
			uint left = size - tail_use;
			memcpy((char*)data+tail_use, MBUF_START(pbuf), left);
		}

		return data;	//get sucess
	}

	return	NULL;		//get failed
}


/*
//if not return NULL that mean there are enough space to alloc though sometimes return a smaller space than wish
void*	mbuf_alloc(mbuf* pbuf, size_t size)
{
	if( pbuf && size > 0 )
	{
		size = MBUF_FIX_ADDR(size);

		if( size <= (uint)MBUF_FREE(pbuf) )
		{
			uint tail_free = MBUF_END(pbuf) - MBUF_TAIL(pbuf) + 1;

			if( size > tail_free )
				size = tail_free;

			char* pret = MBUF_TAIL(pbuf);
			MBUF_TAIL(pbuf) += size;

			if( MBUF_TAIL(pbuf) > MBUF_END(pbuf) )
				MBUF_TAIL(pbuf) = MBUF_START(pbuf);

			*(int*)pret = size;
			
			return pret;
		}
	}
	
	return NULL;
}
*/

void*	mbuf_alloc(mbuf* pbuf, size_t size)
{
	if ( mbuf_tail_free(pbuf) < (int)size )
		mbuf_rewind(pbuf);

	if ( mbuf_tail_free(pbuf) >= (int)size )
	{
		char* ptr = MBUF_TAIL(pbuf);
		MBUF_TAIL(pbuf) += size;

		return ptr;
	}

	return NULL;
}

void	mbuf_rewind(mbuf* pbuf)
{
	int head_free = mbuf_head_free(pbuf);

	if (head_free == 0)
		return;

	memmove(MBUF_START(pbuf), MBUF_HEAD(pbuf), MBUF_USED(pbuf));

	mbuf_head_seek(pbuf, -head_free);
	mbuf_tail_seek(pbuf, -head_free);
}

void	mbuf_clear(mbuf* pbuf)
{
	MBUF_HEAD(pbuf) = MBUF_TAIL(pbuf) = MBUF_START(pbuf);
}

void	mbuf_head_seek(mbuf* pbuf, int offset)
{
	MBUF_HEAD(pbuf) += offset;
}

void	mbuf_tail_seek(mbuf* pbuf, int offset)
{
	MBUF_TAIL(pbuf) += offset;
}

int		mbuf_used(mbuf* pbuf)
{
	return MBUF_USED(pbuf);
}

int		mbuf_total_free(mbuf* pbuf)
{
	return MBUF_FREE(pbuf);
}

int		mbuf_tail_free(mbuf* pbuf)
{
	return MBUF_END(pbuf) - MBUF_TAIL(pbuf) + 1;
}

int		mbuf_head_free(mbuf* pbuf)
{
	return MBUF_HEAD(pbuf) - MBUF_START(pbuf);
}

mbuf*	mbuf_realloc(mbuf* pbuf, size_t size)
{
	uint total_size = MBUF_SIZE(pbuf);
	if ( total_size == size )
		return pbuf;

	uint tail_pos = MBUF_TAIL(pbuf) - MBUF_START(pbuf);
	if ( tail_pos > size )
	{
		mbuf_rewind(pbuf);
		tail_pos = MBUF_TAIL(pbuf) - MBUF_START(pbuf);
		size = tail_pos > size ? tail_pos : size;
	}

	uint head_pos = MBUF_HEAD(pbuf) - MBUF_START(pbuf);
	mbuf* new_buf = (mbuf*)realloc(pbuf, sizeof(mbuf) + size);

    MBUF_SIZE(new_buf) = size;
	MBUF_HEAD(new_buf) += head_pos;
	MBUF_TAIL(new_buf) += tail_pos;

	return new_buf;
}

void*	mbuf_get_head(mbuf* pbuf)
{
	return MBUF_HEAD(pbuf);
}

void*	mbuf_get_tail(mbuf* pbuf)
{
	return MBUF_TAIL(pbuf);
}

int     mbuf_size(mbuf* pbuf)
{
    return MBUF_SIZE(pbuf);
}
