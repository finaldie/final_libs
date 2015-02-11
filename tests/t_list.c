#include <fcunit.h>

#include "flibs/flist.h"

typedef struct {
    int i;
} tnode;

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
    FCUNIT_ASSERT(100 == tn->i);
    return 0;
}

static
int cmp(void *a, void *b)
{
    return *(int*)a - *(int*)b;
}

void test_list_sort()
{
    flist* plist = flist_create();
    int input[] = {2,6,5,3,1,7,8,10,8,9,4};
    int output[] = {1,2,3,4,5,6,7,8,8,9,10};
    int i;
    for(i=0; i<(int)(sizeof(input)/sizeof(int)); ++i)
    {
        flist_push(plist, &input[i]);
    }

    flist_sort(plist, cmp);
    flist_iter it = flist_new_iter(plist);
    for(i=0; i<(int)(sizeof(input)/sizeof(int)); ++i)
    {
        int x = *(int*)flist_each(&it);
        FCUNIT_ASSERT(x == output[i]);
    }
    FCUNIT_ASSERT(output[i-1] == *(int *)plist->tail->data);
    flist_delete(plist);
}
void test_list()
{
    flist* plist = flist_create();
    FCUNIT_ASSERT(plist != NULL);
    int is_empty = flist_empty(plist);
    FCUNIT_ASSERT(1 == is_empty);

    tnode* tn = (tnode*)calloc(1, sizeof(tnode));
    tn->i = 100;
    tnode* tn1 = (tnode*)calloc(1, sizeof(tnode));
    tn1->i = 200;

    {
        int ret = flist_push(plist, tn);
        FCUNIT_ASSERT(0 == ret);

        tnode* thead = (tnode*)flist_head(plist);
        FCUNIT_ASSERT(100 == thead->i);

        is_empty = flist_empty(plist);
        FCUNIT_ASSERT(0 == is_empty);

        ret = flist_push(plist, tn1);
        FCUNIT_ASSERT(0 == ret);

        thead = (tnode*)flist_head(plist);
        FCUNIT_ASSERT(100 == thead->i);

        thead = (tnode*)flist_tail(plist);
        FCUNIT_ASSERT(200 == thead->i);

        tnode* tp = (tnode*)flist_pop(plist);
        FCUNIT_ASSERT(100 == tp->i);

        tnode* tp1 = (tnode*)flist_pop(plist);
        FCUNIT_ASSERT(200 == tp1->i);

        is_empty = flist_empty(plist);
        FCUNIT_ASSERT(1 == is_empty);
    }

    {
        flist_iter it = flist_new_iter(plist);

        int ret = flist_push(plist, tn);
        FCUNIT_ASSERT(0 == ret);

        tnode* t = NULL;
        while( (t = (tnode*)flist_each(&it)) ){
            FCUNIT_ASSERT(100 == t->i);
        }

        flist_pop(plist);
    }

    free(tn);
    free(tn1);

    flist_delete(plist);
}

int main(int argc, char** argv)
{
    FCUNIT_RUN(test_list);
    FCUNIT_RUN(test_list_sort);

    return 0;
}
