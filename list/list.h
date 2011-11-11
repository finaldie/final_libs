//base info: create by hyz
/*effect: lockfree fifo list
*
*
*/


#ifndef _LIST_H_
#define _LIST_H_

typedef struct
{
	void* begin;
	void* end;
}liter;

typedef struct list_mgr lmgr, *pl_mgr;

// return 0 continue iter.. else break out
typedef int	(*plist_call_back)(void* data);

pl_mgr	create_list();
void	delete_list(pl_mgr);

int		push_node(pl_mgr, void* data);
void*	pop_node(pl_mgr);
void*	list_head(pl_mgr);
int		is_empty(pl_mgr);

void*	list_foreach(pl_mgr, plist_call_back);
liter	list_iter(pl_mgr);
void*	list_each(liter*);

#endif
