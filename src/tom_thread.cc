#include "tom_thread.hh"

namespace tom
{

void work_queue_add_entry(WorkQueue *queue, WorkQueueCallback *callback, void *data)
{
    TOM_ASSERT(queue->entry_cnt < ARR_CNT(queue->entries));
    WorkQueueEntry *entry = queue->entries + queue->entry_cnt;
    entry->callback       = callback;
    entry->data           = data;
    _WriteBarrier();
    _mm_sfence();
    // TODO: use InterlockedCompareExchange()
    ++queue->entry_cnt;
    ReleaseSemaphore(queue->semaphore, 1, NULL);
}

internal bool work_queue_do_next_work_entry(WorkQueue *queue)
{
    bool should_sleep = false;

    u32 old_next_entry_to_do = queue->next_entry_to_do;
    if (queue->next_entry_to_do < queue->entry_cnt) {
        {
            u32 index = InterlockedCompareExchange((volatile long *)&queue->next_entry_to_do,
                                                   old_next_entry_to_do + 1, old_next_entry_to_do);
            if (index == old_next_entry_to_do) {
                WorkQueueEntry *entry = queue->entries + index;
                entry->callback(entry->data);

                InterlockedIncrement((volatile long *)&queue->entry_cmp_cnt);
            }
        }
    } else {
        should_sleep = true;
    }

    return should_sleep;
}

void work_queue_complete_all_work(WorkQueue *queue)
{
    WorkQueueEntry entry {};
    while (queue->entry_cnt != queue->entry_cmp_cnt) {
        work_queue_do_next_work_entry(queue);
    }

    queue->entry_cnt        = 0;
    queue->entry_cmp_cnt    = 0;
    queue->next_entry_to_do = 0;
}

bool work_queue_in_progress(WorkQueue *queue)
{
    return queue->entry_cnt == queue->entry_cmp_cnt;
}

internal DWORD WINAPI thread_proc(LPVOID lpParameter)
{
    auto thread = (ThreadInfo *)lpParameter;

    for (;;) {
        if (work_queue_do_next_work_entry(thread->queue)) {
            WaitForSingleObjectEx(thread->queue->semaphore, INFINITE, FALSE);
        }
    }
}

void spawn_threads(ThreadInfo *threads, WorkQueue *queue)
{
    u32 thread_cnt   = THREAD_CNT;
    queue->semaphore = CreateSemaphoreExW(NULL, 0, thread_cnt, NULL, NULL, SEMAPHORE_ALL_ACCESS);

    for (i32 thread_i = 0; thread_i < THREAD_CNT; ++thread_i) {
        threads[thread_i].logical_thread_i = thread_i;
        threads[thread_i].queue            = queue;

        DWORD thread_id;
        HANDLE thread_hnd = CreateThread(0, 0, thread_proc, &threads[thread_i], 0, &thread_id);
        CloseHandle(thread_hnd);
    }
}

}  // namespace tom
