//base info: create by hyz
//effect: fifo list


#include <stdio.h>
#include <stdlib.h>

#include "list.h"

#define	LIST_HEAD(pmgr) 	(pmgr->head)
#define	LIST_TAIL(pmgr)		(pmgr->tail)

//TODO...
struct list_node
{
	void*		data;
	struct list_node*	pre;
	struct list_node*	next;
};

typedef struct list_node lnode, *pnode;

struct list_mgr
{
	struct list_node*	head;
	struct list_node*	tail;
};

pl_mgr	create_list()
{
	pl_mgr pmgr = (pl_mgr)malloc(sizeof(lmgr));
	pnode  node = (pnode)malloc(sizeof(lnode));
	node->pre = node->next = NULL;
	pmgr->head = pmgr->tail = node;

	return pmgr;
}

void	delete_list(pl_mgr pmgr)
{
	while( pop_node(pmgr) );
	free(pmgr->head);
	free(pmgr);
}

int		push_node(pl_mgr pmgr, void* data)
{
	if( !data )
		return 1;

	pnode node = (pnode)malloc(sizeof(lnode));
	node->data = data;
	node->next = NULL;
	node->pre  = pmgr->tail;
	pmgr->tail->next = node;
	pmgr->tail = node;

	return 0;
}

void*	pop_node(pl_mgr pmgr)
{
	if( !pmgr || is_empty(pmgr) )
		return NULL;

	pmgr->head = pmgr->head->next;
	free(pmgr->head->pre);
	return pmgr->head->data;
}

int		is_empty(pl_mgr pmgr)
{
	if(pmgr->head == pmgr->tail)
		return 1;
	return 0;
}

void*	list_foreach(pl_mgr pmgr, plist_call_back pfunc)
{
	if( !pfunc ) return NULL;
	if( is_empty(pmgr) ) return NULL;

	pnode node = LIST_HEAD(pmgr)->next;
	while( node )
	{
		if( pfunc(node->data) )
			break;

		node = node->next;
	}

	return NULL;
}

inline
liter	list_iter(pl_mgr pmgr)
{
	liter iter;

	iter.begin = (void*)pmgr->head;
	iter.end = (void*)pmgr->tail;

	return iter;
}

void*	list_each(liter* iter)
{
	pl_mgr pmgr = (pl_mgr)iter;
	if( is_empty(pmgr) ) return NULL;
	
	pnode node = LIST_HEAD(pmgr)->next;

	if( !node ) return NULL;

	void* data = node->data;
	LIST_HEAD(pmgr) = node;

	return data;
}

// delete current iter point node
// it's not thread safe 
// and break lockfree fifo rules
// so if you want run as safe
// please make sure the push thread and pop thread are same thread
void	list_del(liter* iter)
{
	pl_mgr pmgr = (pl_mgr)iter;
	if( is_empty(pmgr) ) return;

	//pnode curr_node = LIST_HEAD(pmgr);
	//if ( curr_node == LIST_TAIL(pmgr) )
}

void*	list_head(pl_mgr pmgr){
	if( is_empty(pmgr) ) return NULL;

	return LIST_HEAD(pmgr)->next->data;
}

/*
int	main(int argc, char** argv)
{
	int n[10];
	for(int i=0; i<10; ++i)
	{
		n[i] = i;
	}

	pl_mgr pmgr = create_list();

	for(int i=0; i<10; ++i)
		push_node(pmgr, &n[i]);

	for(int i=0; i<10; ++i)
	{
		void* data = pop_node(pmgr);
		if( data )
			printf("%d\n", *(int*)data);
	}

	delete_list(pmgr);

	return 0;
}
*/
