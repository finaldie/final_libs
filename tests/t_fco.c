#include <fcunit.h>
#include "flibs/fco.h"

static
void* test2(fco* co, void* arg)
{
    (void)co;
    (void)arg;
    //printf("[in co2]\n");
    return NULL;
}

static
void* test(fco* co, void* arg)
{
    FCUNIT_ASSERT( co != NULL );
    int n = *(int*)arg;
    //printf("[in co1] arg=%d\n", n);
    FCUNIT_ASSERT(n == 1);
    int n1 = 10;
    int* n2 = NULL;
    n2 = fco_yield(co, &n1);
    //printf("[in co1] after yield n2=%d\n", *n2);
    FCUNIT_ASSERT(*n2 == 100);

    //printf("[in co1] create co2\n");
    fco* co2 = fco_create(co, test2, FCO_TYPE_ALONE);
    FCUNIT_ASSERT(co2 != NULL);
    fco_resume(co2, NULL);
    //printf("[in co1] end\n");
    return NULL;
}

void test_fco()
{
    fco_sched* sched = fco_scheduler_create();
    FCUNIT_ASSERT(sched != NULL);
    fco* co = fco_main_create(sched, test);
    FCUNIT_ASSERT(co != NULL);
    int n = 1;
    int* n1 = NULL;
    n1 = fco_resume(co, &n);
    //printf("[in main] after resume1 n1=%d\n", *n1);
    FCUNIT_ASSERT(*n1 == 10);
    int n2 = 100;
    fco_resume(co, &n2);
    fco_scheduler_destroy(sched);
    //printf("[in main] after resume2\n");
}

int main(int argc, char** argv)
{
    FCUNIT_RUN(test_fco);

    return 0;
}
