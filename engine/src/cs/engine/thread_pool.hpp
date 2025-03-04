// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/math/math.hpp"
#include "cs/engine/event.hpp"
#include "cs/memory/weak_ptr.hpp"
#include "cs/engine/singleton.hpp"
#include "cs/containers/dynamic_array.hpp"

#include <queue>
#include <mutex>
#include <thread>
#include <functional>
#include <condition_variable>

class Thread_Pool : public Singleton<Thread_Pool>
{
public:
    Thread_Pool(uint32 num_threads)
    {
        const uint32 max_threads = std::thread::hardware_concurrency();
        num_threads = clamp(num_threads, 1u, max_threads);

        printf("Initializing thread pool with %d threads. \n", num_threads);

        for (uint32 t = 0; t < num_threads; ++t)
        {
            _workers.emplace_back(std::bind(&Thread_Pool::thread_pool_worker, this));
        }
    }

    void thread_pool_worker()
    {
        while (true) 
        {
            std::function<void()> next_task;

            {
                std::unique_lock<std::mutex> lock(_queue_mutex);
                _condition.wait(lock, [this] { return _should_stop || !_task_queue.empty(); });
                
                if (_should_stop && _task_queue.empty())
                {
                    return;
                }

                next_task = std::move(_task_queue.front());
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

    ~Thread_Pool()
    {
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

    void submit(std::function<void()> task)
    {
        {
            std::unique_lock<std::mutex> lock(_queue_mutex);
            _task_queue.push_back(std::move(task));
        }

        _condition.notify_one();
    }

    void wait_for_completion()
    {
        // int32 previous_num = -1;
        while (!_task_queue.empty())
        {
            // if (_task_queue.size() != previous_num)
            // {
            //     previous_num = _task_queue.size();
            //     printf("remaining tasks: %d\n", previous_num);
            // }
            std::this_thread::yield();
        }

        // printf("No more!\n", previous_num);
    }

private:
    std::vector<std::thread> _workers; //TODO: Make own unique ptr
    // Dynamic_Array<std::thread> _workers; // TODO: introduce emplace resizing for std::thread/unique_ptr (deleted move and copy)
    std::deque<std::function<void()>> _task_queue;    //TODO: Make own queue container
    std::mutex _queue_mutex;
    std::condition_variable _condition;
    std::atomic<bool> _should_stop;
};
