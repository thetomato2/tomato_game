#ifndef TOM_THREAD_HH
#define TOM_THREAD_HH

#include "tom_core.hh"

namespace tom
{

#define THREAD_CNT 8

////////////////////////////////////////////////////////////////////////////////////////////////
// #Thead Types
struct WorkQueue;
// typedef void WorkQueueCallback(WorkQueue*, void*);
typedef void WorkQueueCallback(void *);

struct WorkQueueEntry
{
    WorkQueueCallback *callback;
    void *data;
};

struct WorkQueue
{
    volatile u32 next_entry_to_do;
    volatile u32 entry_cnt;
    volatile u32 entry_cmp_cnt;
    HANDLE semaphore;
    WorkQueueEntry entries[256];
};

struct ThreadInfo
{
    WorkQueue *queue;
    i32 logical_thread_i;
};

////////////////////////////////////////////////////////////////////////////////////////////////
// #Thead Functions
void work_queue_add_entry(WorkQueue *queue, WorkQueueCallback *callback, void *data);
void work_queue_complete_all_work(WorkQueue *queue);
bool work_queue_in_progress(WorkQueue *queue);
void spawn_threads(ThreadInfo *threads, WorkQueue *queue);

}  // namespace tom

#endif  // !TOM_THREAD_HH
