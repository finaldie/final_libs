#include "flibs/ftu_inc.h"
#include "flibs/fco.h"
#include "inc.h"

static
void* test2(fco* co, void* arg)
{
    (void)co;
    (void)arg;
    printf("[in co2]\n");
    return NULL;
}

static
void* test(fco* co, void* arg)
{
    FTU_ASSERT( co != NULL );
    int n = *(int*)arg;
    printf("[in co1] arg=%d\n", n);
    int n1 = 10;
    int* n2 = fco_yield(co, &n1);
    printf("[in co1] after yield n2=%d\n", *n2);
    FTU_ASSERT(*n2 == 100);

    printf("[in co1] create co2\n");
    fco* co2 = fco_create(co, test2, FCO_TYPE_ALONE);
    FTU_ASSERT( co2 != NULL );
    fco_resume(co2, NULL);
    printf("[in co1] end\n");
    return NULL;
}

void test_fco()
{
    fco_sched* sched = fco_scheduler_create();
    FTU_ASSERT(sched != NULL);
    fco* co = fco_main_create(sched, test);
    FTU_ASSERT(co != NULL);
    int n = 1;
    int* n1 = fco_resume(co, &n);
    printf("[in main] after resume1 n1=%d\n", *n1);
    FTU_ASSERT(*n1 == 10);
    int n2 = 100;
    fco_resume(co, &n2);
    fco_scheduler_destroy(sched);
    printf("[in main] after resume2\n");

}
