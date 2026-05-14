// Stress test for codexion scheduler: verifies EDF/FIFO ordering, no deadlock, no starvation.
// Compile:
// gcc tests/stress_scheduler.c src/scheduler/scheduler.c src/scheduler/scheduler_sync.c src/scheduler/heap.c -Iinclude -lpthread -o stress_test

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "codexion.h"

#define N_THREADS 48
#define TIMEOUT_SEC 5

typedef struct s_result
{
    int             order[N_THREADS];
    int             order_count;
    int             completed[N_THREADS];
    pthread_mutex_t lock;
}   t_result;

typedef struct s_worker_arg
{
    t_context    *ctx;
    t_result     *result;
    int           coder_id;
}   t_worker_arg;

typedef struct s_watchdog_arg
{
    t_context *ctx;
    int       *timed_out;
}   t_watchdog_arg;

typedef struct s_expected
{
    int  coder_id;
    long deadline_ms;
}   t_expected;

static int  cmp_expected(const void *a, const void *b)
{
    const t_expected *ea;
    const t_expected *eb;

    ea = (const t_expected *)a;
    eb = (const t_expected *)b;
    if (ea->deadline_ms < eb->deadline_ms)
        return (-1);
    if (ea->deadline_ms > eb->deadline_ms)
        return (1);
    if (ea->coder_id < eb->coder_id)
        return (-1);
    if (ea->coder_id > eb->coder_id)
        return (1);
    return (0);
}

static void *worker(void *arg)
{
    t_worker_arg *a;
    int           ok;
    int           idx;

    a = (t_worker_arg *)arg;
    ok = scheduler_wait_turn(a->ctx, a->coder_id);
    if (!ok)
        return (NULL);
    pthread_mutex_lock(&a->result->lock);
    idx = a->result->order_count;
    a->result->order[idx] = a->coder_id;
    a->result->order_count++;
    a->result->completed[a->coder_id] = 1;
    pthread_mutex_unlock(&a->result->lock);
    return (NULL);
}

static void *watchdog(void *arg)
{
    t_watchdog_arg *a;

    a = (t_watchdog_arg *)arg;
    sleep(TIMEOUT_SEC);
    pthread_mutex_lock(&a->ctx->scheduler_mutex);
    if (!a->ctx->simulation_over && !scheduler_is_empty(&a->ctx->scheduler_heap))
    {
        a->ctx->simulation_over = 1;
        *a->timed_out = 1;
        pthread_cond_broadcast(&a->ctx->scheduler_cond);
    }
    pthread_mutex_unlock(&a->ctx->scheduler_mutex);
    return (NULL);
}

static void test_init_context(t_context *ctx, t_policy policy)
{
    memset(ctx, 0, sizeof(*ctx));
    scheduler_init(&ctx->scheduler_heap, N_THREADS * 2, policy);
    pthread_mutex_init(&ctx->scheduler_mutex, NULL);
    pthread_cond_init(&ctx->scheduler_cond, NULL);
    ctx->simulation_over = 0;
    ctx->next_seq_no = 0;
}

static void test_destroy_context(t_context *ctx)
{
    scheduler_destroy(&ctx->scheduler_heap);
    pthread_mutex_destroy(&ctx->scheduler_mutex);
    pthread_cond_destroy(&ctx->scheduler_cond);
}

static void fill_jobs_and_expected(
    t_context *ctx,
    t_policy policy,
    long deadlines[N_THREADS],
    int expected_order[N_THREADS])
{
    t_expected expected[N_THREADS];
    int        i;

    for (i = 0; i < N_THREADS; i++)
    {
        if (policy == POLICY_FIFO)
            deadlines[i] = 1000;
        else
            deadlines[i] = (long)((N_THREADS - i) * 7 + (i % 3));
        scheduler_request_slot(ctx, i, deadlines[i]);
        expected[i].coder_id = i;
        expected[i].deadline_ms = deadlines[i];
    }
    if (policy == POLICY_FIFO)
    {
        for (i = 0; i < N_THREADS; i++)
            expected_order[i] = i;
        return ;
    }
    qsort(expected, N_THREADS, sizeof(t_expected), cmp_expected);
    for (i = 0; i < N_THREADS; i++)
        expected_order[i] = expected[i].coder_id;
}

static void run_test(t_policy policy)
{
    t_context       ctx;
    pthread_t       threads[N_THREADS];
    t_worker_arg    args[N_THREADS];
    pthread_t       wd_thread;
    t_watchdog_arg  wd_arg;
    t_result        result;
    int             expected_order[N_THREADS];
    long            deadlines[N_THREADS];
    int             timed_out;
    int             i;

    test_init_context(&ctx, policy);
    memset(&result, 0, sizeof(result));
    pthread_mutex_init(&result.lock, NULL);
    fill_jobs_and_expected(&ctx, policy, deadlines, expected_order);
    for (i = 0; i < N_THREADS; i++)
    {
        args[i].ctx = &ctx;
        args[i].result = &result;
        args[i].coder_id = i;
        pthread_create(&threads[i], NULL, worker, &args[i]);
    }
    timed_out = 0;
    wd_arg.ctx = &ctx;
    wd_arg.timed_out = &timed_out;
    pthread_create(&wd_thread, NULL, watchdog, &wd_arg);
    for (i = 0; i < N_THREADS; i++)
        pthread_join(threads[i], NULL);
    pthread_mutex_lock(&ctx.scheduler_mutex);
    ctx.simulation_over = 1;
    pthread_cond_broadcast(&ctx.scheduler_cond);
    pthread_mutex_unlock(&ctx.scheduler_mutex);
    pthread_join(wd_thread, NULL);

    assert(!timed_out && "Deadlock detected by watchdog");
    assert(result.order_count == N_THREADS && "Starvation detected: not all threads completed");
    for (i = 0; i < N_THREADS; i++)
        assert(result.completed[i] && "Starvation detected: at least one coder did not complete");
    for (i = 0; i < N_THREADS; i++)
        assert(result.order[i] == expected_order[i] && "Scheduling order violated");

    if (policy == POLICY_FIFO)
        printf("FIFO stress test passed (%d threads)\n", N_THREADS);
    else
        printf("EDF stress test passed (%d threads)\n", N_THREADS);

    pthread_mutex_destroy(&result.lock);
    test_destroy_context(&ctx);
}

int main(void)
{
    printf("Running FIFO stress test...\n");
    run_test(POLICY_FIFO);
    printf("Running EDF stress test...\n");
    run_test(POLICY_EDF);
    printf("All stress tests passed!\n");
    return (0);
}
