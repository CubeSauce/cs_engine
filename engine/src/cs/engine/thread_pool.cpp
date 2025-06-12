// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/engine/thread_pool.hpp"

template<> 
Thread_Pool* Singleton<Thread_Pool>::_singleton { nullptr };
// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/math/math.hpp"
#include "cs/engine/event.hpp"
#include "../../../third_party/cs_util/src/cs/memory/weak_ptr.hpp"
#include "cs/engine/singleton.hpp"
#include "cs/engine/profiling/profiler.hpp"
#include "cs/containers/dynamic_array.hpp"
#include "cs/engine/task_system.hpp"

#include <queue>
#include <mutex>
#include <thread>
#include <functional>
#include <condition_variable>

Thread_Pool::Thread_Pool(uint32 num_threads)
    : _num_threads(clamp(num_threads, 0u, std::thread::hardware_concurrency()))
{
    printf("Initializing thread pool with %d threads. \n", _num_threads);

    for (uint32 t = 0; t < _num_threads; ++t)
    {
        _workers.emplace_back(std::bind(&Thread_Pool::_thread_pool_worker, this));
    }
}

Thread_Pool::~Thread_Pool()
{
    if (_num_threads == 0)
    {
        return;
    }

    {
        std::unique_lock<std::mutex> lock(_queue_mutex);
        _should_stop = true;
    }

    _condition.notify_all();

    for (std::thread &worker : _workers)
    {
        worker.join();
    }
}

void Thread_Pool::submit(const Dynamic_Array<Shared_Ptr<Task>>& tasks)
{
    if (_num_threads == 0)
    {
        for (const Shared_Ptr<Task>& task : tasks)
        {
            task->execute_on_this_thread();
        }

        return;
    }

    {
        std::unique_lock<std::mutex> lock(_queue_mutex);
        for (const Shared_Ptr<Task>& task : tasks)
        {
            _task_queue.push_back(task);
        }
    }

    _condition.notify_all();
}

void Thread_Pool::wait_for_completion()
{
    PROFILE_FUNCTION()

    while (!_task_queue.empty())
    {
        std::this_thread::yield();
    }
}

void Thread_Pool::_thread_pool_worker()
{
    PROFILE_FUNCTION()

    Shared_Ptr<Task> current_task;

    while (true) 
    {
        std::function<void()> next_task;

        {
            Scoped_Profiler("Waiting on mutex");

            std::unique_lock<std::mutex> lock(_queue_mutex);
            _condition.wait(lock, [this] { return _should_stop || !_task_queue.empty(); });
            
            if (_should_stop && _task_queue.empty())
            {
                return;
            }

            current_task = _task_queue.front();
            if (current_task.is_valid())
            {
                next_task = std::move(current_task->get_binding());
            }
            _task_queue.pop_front();
        }
        
        //printf("--------- Thread %d: \n---------------\n", tid);
        if (next_task)
        {
            next_task(); // Execute the task
        }
        //printf("--------------------------------------\n");
    }
}
