// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/math/math.hpp"
#include "cs/engine/event.hpp"
#include "cs/memory/weak_ptr.hpp"
#include "cs/engine/singleton.hpp"
#include "cs/engine/profiling/profiler.hpp"
#include "cs/containers/dynamic_array.hpp"

#include <queue>
#include <mutex>
#include <thread>
#include <functional>
#include <condition_variable>

class Task;
class Thread_Pool : public Singleton<Thread_Pool>
{
public:
    Thread_Pool(uint32 num_threads);
    ~Thread_Pool();

    void submit(const Dynamic_Array<Shared_Ptr<Task>>& tasks);
    void wait_for_completion();

private:
    std::vector<std::thread> _workers; //TODO: Make own unique ptr
    // Dynamic_Array<std::thread> _workers; // TODO: introduce emplace resizing for std::thread/unique_ptr (deleted move and copy)
    std::deque<Shared_Ptr<Task>> _task_queue;    //TODO: Make own queue container
    std::mutex _queue_mutex;
    std::condition_variable _condition;
    std::atomic<bool> _should_stop;

private:
    void _thread_pool_worker();
};
