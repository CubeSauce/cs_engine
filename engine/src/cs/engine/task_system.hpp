// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/engine/event.hpp"
#include "cs/memory/weak_ptr.hpp"
#include "cs/containers/dynamic_array.hpp"
#include "cs/engine/thread_pool.hpp"

#include <atomic>
#include <functional>

class Task : public Shared_From_This<Task>
{
public:
    using Task_Job = std::function<void(void)>;

    Task(const Task_Job& job);

    void add_dependency(const Shared_Ptr<Task>& task);
    void execute();
    void reset();

    bool has_unfinished_dependencies() const;
    bool has_executed() const;
    bool can_execute() const;

protected:
    Task_Job _job;
    Dynamic_Array<Shared_Ptr<Task>> _dependencies;
    Dynamic_Array<Weak_Ptr<Task>> _references;
    std::atomic<int32> _unfinished_dependencies { 0 };
    bool _has_executed { false };
};

class Task_Graph
{
public:
    Shared_Ptr<Task> create_task(std::function<void(void)> task_job);

    void execute();
    void reset();

private:
    Dynamic_Array<Shared_Ptr<Task>> _tasks;
};
