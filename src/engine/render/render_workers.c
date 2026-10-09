#include "render_workers.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "3dc.h"
#include "module.h"
#include "kshape.h"

extern unsigned char GammaValues[256];

#if defined(_WIN32)
#include <windows.h>
typedef HANDLE avp_thread_t;
typedef CRITICAL_SECTION avp_mutex_t;
typedef CONDITION_VARIABLE avp_cond_t;
static void avp_mutex_init(avp_mutex_t *m) { InitializeCriticalSection(m); }
static void avp_mutex_destroy(avp_mutex_t *m) { DeleteCriticalSection(m); }
static void avp_mutex_lock(avp_mutex_t *m) { EnterCriticalSection(m); }
static void avp_mutex_unlock(avp_mutex_t *m) { LeaveCriticalSection(m); }
static void avp_cond_init(avp_cond_t *c) { InitializeConditionVariable(c); }
static void avp_cond_signal(avp_cond_t *c) { WakeConditionVariable(c); }
static void avp_cond_broadcast(avp_cond_t *c) { WakeAllConditionVariable(c); }
static void avp_cond_wait(avp_cond_t *c, avp_mutex_t *m) { SleepConditionVariableCS(c, m, INFINITE); }
#else
#include <pthread.h>
#include <unistd.h>
typedef pthread_t avp_thread_t;
typedef pthread_mutex_t avp_mutex_t;
typedef pthread_cond_t avp_cond_t;
static void avp_mutex_init(avp_mutex_t *m) { pthread_mutex_init(m, NULL); }
static void avp_mutex_destroy(avp_mutex_t *m) { pthread_mutex_destroy(m); }
static void avp_mutex_lock(avp_mutex_t *m) { pthread_mutex_lock(m); }
static void avp_mutex_unlock(avp_mutex_t *m) { pthread_mutex_unlock(m); }
static void avp_cond_init(avp_cond_t *c) { pthread_cond_init(c, NULL); }
static void avp_cond_signal(avp_cond_t *c) { pthread_cond_signal(c); }
static void avp_cond_broadcast(avp_cond_t *c) { pthread_cond_broadcast(c); }
static void avp_cond_wait(avp_cond_t *c, avp_mutex_t *m) { pthread_cond_wait(c, m); }
#endif

typedef struct AVP_RenderJob
{
    size_t input_offset;
    size_t count;
    AVP_GLVertex *output;
    AVP_RenderVertexMode mode;
    AVP_RenderVertexJobParams params;
} AVP_RenderJob;

typedef struct AVP_RenderBatch
{
    RENDERVERTEX input[256];
    AVP_RenderJob jobs[32];
    size_t job_count;
    size_t vertex_count;
} AVP_RenderBatch;

#define AVP_RENDER_QUEUE_SIZE 64
#define AVP_RENDER_MAX_WORKERS 4
#define AVP_RENDER_BATCH_VERTICES 256
#define AVP_RENDER_BATCH_JOBS 32
#define AVP_RENDER_ASYNC_MIN_VERTICES 32
#define AVP_RENDER_ASYNC_MIN_JOBS 4

static AVP_RenderBatch queue[AVP_RENDER_QUEUE_SIZE];
static size_t queue_head, queue_tail, queue_count;
static size_t active_batches;
static int stopping;
static int enabled;
static avp_thread_t workers[AVP_RENDER_MAX_WORKERS];
static size_t worker_count;
static avp_mutex_t queue_mutex;
static avp_cond_t work_cond;
static avp_cond_t space_cond;
static avp_cond_t idle_cond;
static int initialized;
static AVP_RenderBatch pending_batch;

static void prepare_job(const AVP_RenderBatch *batch, const AVP_RenderJob *job)
{
    size_t i;
    const float proj_x = (job->params.proj_x + 1.0f) / job->params.centre_x;
    const float proj_y = (job->params.proj_y + 1.0f) / job->params.centre_y;

    for (i = 0; i < job->count; ++i)
    {
        const RENDERVERTEX *src = &batch->input[job->input_offset + i];
        AVP_GLVertex *dst = &job->output[i];
        const float w = (float)src->Z;
        const float x = (float)src->X * proj_x;
        const float y = -(float)src->Y * proj_y;
        float z;

        dst->v[0] = x;
        dst->v[1] = y;

        switch (job->mode)
        {
        case AVP_RENDER_VERTEX_SKY:
            dst->v[2] = w;
            break;
        case AVP_RENDER_VERTEX_PARTICLE:
            if (job->params.flags & 1)
                z = -0.99999f;
            else if (job->params.flags & 2)
                z = 0.99999f;
            else
                z = 1.0f - 2.0f * job->params.z_near / w;
            dst->v[2] = z * w;
            break;
        default:
            z = 1.0f - 2.0f * job->params.z_near /
                           (w + job->params.hud_z_offset);
            dst->v[2] = z * w;
            break;
        }

        dst->v[3] = w;
        dst->t[0] = 0.0f;
        dst->t[1] = 0.0f;

        if (job->mode == AVP_RENDER_VERTEX_GOURAUD_TEXTURED ||
            job->mode == AVP_RENDER_VERTEX_CLOAKED ||
            job->mode == AVP_RENDER_VERTEX_SKY ||
            job->mode == AVP_RENDER_VERTEX_DECAL ||
            job->mode == AVP_RENDER_VERTEX_PARTICLE)
        {
            dst->t[0] = ((float)src->U) * job->params.recip_w;
            dst->t[1] = ((float)src->V) * job->params.recip_h;
        }

        switch (job->mode)
        {
        case AVP_RENDER_VERTEX_GOURAUD_TEXTURED:
            dst->c[0] = GammaValues[src->R];
            dst->c[1] = GammaValues[src->G];
            dst->c[2] = GammaValues[src->B];
            dst->c[3] = src->A;
            dst->s[0] = GammaValues[src->SpecularR];
            dst->s[1] = GammaValues[src->SpecularG];
            dst->s[2] = GammaValues[src->SpecularB];
            dst->s[3] = src->A;
            break;
        case AVP_RENDER_VERTEX_GOURAUD:
            dst->c[0] = src->R;
            dst->c[1] = src->G;
            dst->c[2] = src->B;
            dst->c[3] = (job->params.flags & 4) ? src->A : 255;
            memset(dst->s, 0, sizeof(dst->s));
            break;
        case AVP_RENDER_VERTEX_THERMAL:
        case AVP_RENDER_VERTEX_CLOAKED:
        case AVP_RENDER_VERTEX_SKY:
            dst->c[0] = src->R;
            dst->c[1] = src->G;
            dst->c[2] = src->B;
            dst->c[3] = src->A;
            memset(dst->s, 0, sizeof(dst->s));
            break;
        case AVP_RENDER_VERTEX_DECAL:
        case AVP_RENDER_VERTEX_PARTICLE:
            dst->c[0] = (uint8_t)job->params.color_r;
            dst->c[1] = (uint8_t)job->params.color_g;
            dst->c[2] = (uint8_t)job->params.color_b;
            dst->c[3] = (uint8_t)job->params.color_a;
            memset(dst->s, 0, sizeof(dst->s));
            break;
        }
    }
}

static void prepare_batch(const AVP_RenderBatch *batch)
{
    size_t i;
    for (i = 0; i < batch->job_count; ++i)
        prepare_job(batch, &batch->jobs[i]);
}

#if defined(_WIN32)
static DWORD WINAPI worker_main(LPVOID unused)
#else
static void *worker_main(void *unused)
#endif
{
    (void)unused;
    for (;;)
    {
        AVP_RenderBatch batch;
        avp_mutex_lock(&queue_mutex);
        while (queue_count == 0 && !stopping)
            avp_cond_wait(&work_cond, &queue_mutex);
        if (queue_count == 0 && stopping)
        {
            avp_mutex_unlock(&queue_mutex);
            break;
        }
        batch = queue[queue_head];
        queue_head = (queue_head + 1) % AVP_RENDER_QUEUE_SIZE;
        queue_count--;
        active_batches++;
        avp_cond_signal(&space_cond);
        avp_mutex_unlock(&queue_mutex);

        prepare_batch(&batch);

        avp_mutex_lock(&queue_mutex);
        active_batches--;
        if (queue_count == 0 && active_batches == 0)
            avp_cond_broadcast(&idle_cond);
        avp_mutex_unlock(&queue_mutex);
    }
#if defined(_WIN32)
    return 0;
#else
    return NULL;
#endif
}

static size_t detect_worker_count(void)
{
#if defined(_WIN32)
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    if (info.dwNumberOfProcessors < 2)
        return 0;
    if (info.dwNumberOfProcessors - 1 < AVP_RENDER_MAX_WORKERS)
        return info.dwNumberOfProcessors - 1;
    return AVP_RENDER_MAX_WORKERS;
#else
    long cpus = sysconf(_SC_NPROCESSORS_ONLN);
    if (cpus < 2)
        return 0;
    if (cpus - 1 < AVP_RENDER_MAX_WORKERS)
        return (size_t)(cpus - 1);
    return AVP_RENDER_MAX_WORKERS;
#endif
}

static void enqueue_batch_locked(const AVP_RenderBatch *batch)
{
    AVP_RenderBatch *dst;
    while (queue_count == AVP_RENDER_QUEUE_SIZE && !stopping)
        avp_cond_wait(&space_cond, &queue_mutex);
    if (stopping)
        return;
    dst = &queue[queue_tail];
    *dst = *batch;
    queue_tail = (queue_tail + 1) % AVP_RENDER_QUEUE_SIZE;
    queue_count++;
    avp_cond_signal(&work_cond);
}

static void flush_pending_batch(int force_async)
{
    AVP_RenderBatch batch;
    int async;

    if (pending_batch.job_count == 0)
        return;

    batch = pending_batch;
    memset(&pending_batch, 0, sizeof(pending_batch));

    async = enabled &&
            (force_async ||
             batch.vertex_count >= AVP_RENDER_ASYNC_MIN_VERTICES ||
             batch.job_count >= AVP_RENDER_ASYNC_MIN_JOBS);

    if (!async)
    {
        prepare_batch(&batch);
        return;
    }

    avp_mutex_lock(&queue_mutex);
    enqueue_batch_locked(&batch);
    avp_mutex_unlock(&queue_mutex);
}

void AVP_RenderWorkers_Init(void)
{
    size_t i;
    if (initialized)
        return;

    avp_mutex_init(&queue_mutex);
    avp_cond_init(&work_cond);
    avp_cond_init(&space_cond);
    avp_cond_init(&idle_cond);
    queue_head = queue_tail = queue_count = active_batches = 0;
    stopping = 0;
    enabled = 0;
    memset(&pending_batch, 0, sizeof(pending_batch));

    worker_count = detect_worker_count();
    for (i = 0; i < worker_count; ++i)
    {
#if defined(_WIN32)
        workers[i] = CreateThread(NULL, 0, worker_main, NULL, 0, NULL);
        if (!workers[i])
            break;
#else
        if (pthread_create(&workers[i], NULL, worker_main, NULL) != 0)
            break;
#endif
        enabled = 1;
    }
    worker_count = i;
    initialized = 1;
}

void AVP_RenderWorkers_Shutdown(void)
{
    size_t i;
    if (!initialized)
        return;

    AVP_RenderWorkers_WaitAll();
    avp_mutex_lock(&queue_mutex);
    stopping = 1;
    avp_cond_broadcast(&work_cond);
    avp_mutex_unlock(&queue_mutex);

    for (i = 0; i < worker_count; ++i)
    {
#if defined(_WIN32)
        WaitForSingleObject(workers[i], INFINITE);
        CloseHandle(workers[i]);
#else
        pthread_join(workers[i], NULL);
#endif
    }

    avp_mutex_destroy(&queue_mutex);
    initialized = 0;
    enabled = 0;
    worker_count = 0;
}

void AVP_RenderWorkers_Submit(const void *vertices, size_t count,
                              AVP_GLVertex *output,
                              AVP_RenderVertexMode mode,
                              const AVP_RenderVertexJobParams *params)
{
    AVP_RenderJob *job;

    if (!initialized || !enabled || count == 0 || count > 256)
    {
        AVP_RenderBatch local_batch;
        AVP_RenderJob local_job;
        memset(&local_batch, 0, sizeof(local_batch));
        memset(&local_job, 0, sizeof(local_job));
        memcpy(local_batch.input, vertices, count * sizeof(RENDERVERTEX));
        local_job.input_offset = 0;
        local_job.count = count;
        local_job.output = output;
        local_job.mode = mode;
        local_job.params = *params;
        local_batch.jobs[0] = local_job;
        local_batch.job_count = 1;
        local_batch.vertex_count = count;
        prepare_batch(&local_batch);
        return;
    }

    if (pending_batch.job_count == AVP_RENDER_BATCH_JOBS ||
        pending_batch.vertex_count + count > AVP_RENDER_BATCH_VERTICES)
        flush_pending_batch(1);

    job = &pending_batch.jobs[pending_batch.job_count];
    memset(job, 0, sizeof(*job));
    job->input_offset = pending_batch.vertex_count;
    memcpy(&pending_batch.input[pending_batch.vertex_count],
           vertices, count * sizeof(RENDERVERTEX));
    job->count = count;
    job->output = output;
    job->mode = mode;
    job->params = *params;
    pending_batch.job_count++;
    pending_batch.vertex_count += count;

    if (pending_batch.vertex_count >= AVP_RENDER_BATCH_VERTICES ||
        pending_batch.job_count >= AVP_RENDER_BATCH_JOBS)
        flush_pending_batch(0);
}

void AVP_RenderWorkers_WaitAll(void)
{
    if (!initialized)
        return;

    flush_pending_batch(0);

    if (!enabled)
        return;
    avp_mutex_lock(&queue_mutex);
    while (queue_count != 0 || active_batches != 0)
        avp_cond_wait(&idle_cond, &queue_mutex);
    avp_mutex_unlock(&queue_mutex);
}

int AVP_RenderWorkers_IsEnabled(void)
{
    return enabled;
}
