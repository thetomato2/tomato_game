#include "tom_thread.hh"

namespace tom
{

void work_queue_add_entry(WorkQueue *queue, WorkQueueCallback *callback, void *data)
{
    u32 new_next_entry_to_write = (queue->next_entry_to_write + 1) % ARR_CNT(queue->entries);
    // TOM_ASSERT(new_next_entry_to_write != queue->next_entry_to_read);
    WorkQueueEntry *entry = queue->entries + queue->next_entry_to_write;
    entry->callback       = callback;
    entry->data           = data;
    ++queue->completion_goal;
    _WriteBarrier();
    _mm_sfence();
    // TODO: use InterlockedCompareExchange()
    queue->next_entry_to_write = new_next_entry_to_write;
    ReleaseSemaphore(queue->semaphore, 1, NULL);
}

internal bool work_queue_do_next_work_entry(WorkQueue *queue)
{
    bool should_sleep = false;

    u32 old_next_entry_to_read = queue->next_entry_to_read;
    u32 new_next_entry_to_read = (old_next_entry_to_read + 1) % ARR_CNT(queue->entries);
    if (old_next_entry_to_read != queue->next_entry_to_write) {
        {
            u32 index = InterlockedCompareExchange((volatile long *)&queue->next_entry_to_read,
                                                   new_next_entry_to_read, old_next_entry_to_read);
            if (index == old_next_entry_to_read) {
                WorkQueueEntry *entry = queue->entries + index;
                entry->callback(entry->data);

                InterlockedIncrement((volatile long *)&queue->completion_cnt);
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
    while (queue->completion_goal != queue->completion_cnt) {
        work_queue_do_next_work_entry(queue);
    }

    queue->completion_goal = 0;
    queue->completion_cnt  = 0;
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
