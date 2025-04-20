// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/engine/task_system.hpp"
#include "cs/engine/profiling/profiler.hpp"

Task::Task(const Task::Job &job)
    : _job(job)
{
}

void Task::add_dependency(const Shared_Ptr<Task> &task)
{
    _dependencies.push_back(task);
    task->_references.push_back(shared_from_this());
    _unfinished_dependencies++;
}

void Task::reset()
{
    _unfinished_dependencies = _dependencies.size();
    _has_executed = false;
}

void Task::execute_on_this_thread()
{
    _submit_to_thread_pool();
}

bool Task::has_unfinished_dependencies() const 
{ 
    return _unfinished_dependencies > 0; 
}
bool Task::has_executed() const 
{ 
    return _has_executed; 
}
bool Task::can_execute() const 
{ 
    return !has_unfinished_dependencies() && !has_executed(); 
}

Task::Binding Task::get_binding()
{
    return std::bind(&Task::_submit_to_thread_pool, this);
}

void Task::_submit_to_thread_pool()
{
    _job();
    _has_executed = true;
    
    Dynamic_Array<Shared_Ptr<Task>> referencers_to_execute;
    for (Weak_Ptr<Task> weak_referencer : _references)
    {
        Shared_Ptr<Task> shared_referencer = weak_referencer.lock();
        if (!shared_referencer)
        {
            continue;
        }

        shared_referencer->_unfinished_dependencies--;
        if (!shared_referencer->can_execute())
        { 
            continue;
        }

        referencers_to_execute.push_back(shared_referencer);
    } 

    Thread_Pool::get().submit(referencers_to_execute);
}

Shared_Ptr<Task> Task_Graph::create_task(std::function<void(void)> task_job)
{
    Shared_Ptr<Task> new_task = Shared_Ptr<Task>::create(task_job);
    _tasks.push_back(new_task);
    return new_task;
}

void Task_Graph::execute()
{
    PROFILE_FUNCTION()

    Thread_Pool::get().submit(_tasks);
    Thread_Pool::get().wait_for_completion();
}

void Task_Graph::reset()
{
    for (Shared_Ptr<Task>& task : _tasks)
    {
        if (!task.is_valid())
        {
            continue;
        }

        task->reset();
    }
}

void Task_Graph::clear()
{
    _tasks.clear();
}
