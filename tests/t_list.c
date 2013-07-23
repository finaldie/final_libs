/*
 * =====================================================================================
 *
 *       Filename:  t_list.c
 *
 *    Description:  test case for list
 *
 *        Version:  1.0
 *        Created:  11/25/2011 17:08:55
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  finaldie
 *        Company:  
 *
 * =====================================================================================
 */

#include "ftu_inc.h"
#include "flist.h"
#include "inc.h"

typedef struct {
    int i;
}tnode;
struct list_node {
    void*             data;
    struct list_node* pre;
    struct list_node* next;
};
struct list_mgr {
    struct list_node* head;
    struct list_node* tail;
};
int test_list_foreach(void* data)
{
    tnode* tn = (tnode*)data;
    FTU_ASSERT_EQUAL_INT(100, tn->i);
    return 0;
}
void test_list_sort()
{
	pl_mgr plist = flist_create();
	int input[] = {2,6,5,3,1,7,8,10,8,9,4};
	int output[] = {1,2,3,4,5,6,7,8,8,9,10};
	int i;
	for(i=0; i<sizeof(input)/sizeof(int); ++i)
	{
		flist_push(plist, &input[i]);
	}
	int cmp(void *a, void *b)
	{
		return *(int*)a - *(int*)b;
	}
	flist_sort(plist, cmp);
	liter it = flist_iter(plist);
	for(i=0; i<sizeof(input)/sizeof(int); ++i)
	{
		int x = *(int*)flist_each(&it);
		FTU_ASSERT_EQUAL_INT(x, output[i]);
	}
	FTU_ASSERT_EQUAL_INT(output[i-1], *(int *)plist->tail->data);
	flist_delete(plist);
}
void test_list()
{
    pl_mgr plist = flist_create();
    FTU_ASSERT_EXPRESS(plist!=NULL);
    int is_empty = flist_isempty(plist);
    FTU_ASSERT_EQUAL_INT(1, is_empty);

    tnode* tn = (tnode*)malloc(sizeof(tnode));
    tn->i = 100;
    tnode* tn1 = (tnode*)malloc(sizeof(tnode));
    tn1->i = 200;

    {
        int ret = flist_push(plist, tn);
        FTU_ASSERT_EQUAL_INT(0, ret);

        tnode* thead = (tnode*)flist_head(plist);
        FTU_ASSERT_EQUAL_INT(100, thead->i);

        is_empty = flist_isempty(plist);
        FTU_ASSERT_EQUAL_INT(0, is_empty);
        
        ret = flist_push(plist, tn1);
        FTU_ASSERT_EQUAL_INT(0, ret);

        thead = (tnode*)flist_head(plist);
        FTU_ASSERT_EQUAL_INT(100, thead->i);

        tnode* tp = (tnode*)flist_pop(plist);
        FTU_ASSERT_EQUAL_INT(100, tp->i);

        tnode* tp1 = (tnode*)flist_pop(plist);
        FTU_ASSERT_EQUAL_INT(200, tp1->i);

        is_empty = flist_isempty(plist);
        FTU_ASSERT_EQUAL_INT(1, is_empty);
    }
    
    {
        liter it = flist_iter(plist);

        int ret = flist_push(plist, tn);
        FTU_ASSERT_EQUAL_INT(0, ret);
        
        tnode* t = NULL;
        while( (t = (tnode*)flist_each(&it)) ){
            FTU_ASSERT_EQUAL_INT(100, t->i);
        }

        flist_pop(plist);
    }

    free(tn);
    free(tn1);

    flist_delete(plist);
}
