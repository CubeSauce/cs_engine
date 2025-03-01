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
            workers.emplace_back([&, tid = t](){
                while (true) 
                {
                    std::function<void()> next_task;

                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, [this] { return stop || !tasks.empty(); });
                        
                        if (stop && tasks.empty())
                        {
                            return;
                        }

                        next_task = std::move(tasks.front());
                        tasks.pop_front();
                    }
                    
                    //printf("--------- Thread %d: \n---------------\n", tid);
                    next_task(); // Execute the task
                    //printf("--------------------------------------\n");
                }
            });
        }
    }

    ~Thread_Pool()
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }

        condition.notify_all();

        for (std::thread &worker : workers)
        {
            worker.join();
        }
    }

    void submit(std::function<void()> task)
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.push_back(std::move(task));
        }

        condition.notify_one();
    }

private:
    std::vector<std::thread> workers; //TODO: Make own unique ptr
    // Dynamic_Array<std::thread> _workers; // TODO: introduce emplace resizing for std::thread/unique_ptr (deleted move and copy)
    std::deque<std::function<void()>> tasks;    //TODO: Make own queue container
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
};
