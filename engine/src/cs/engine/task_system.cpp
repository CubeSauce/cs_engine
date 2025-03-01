// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/engine/task_system.hpp"

Task::Task(const Task::Task_Job &job)
    : _job(job)
{
}

void Task::add_dependency(const Shared_Ptr<Task> &task)
{
    _dependencies.add(task);
    task->_references.add(shared_from_this());
    _unfinished_dependencies++;
}

void Task::execute()
{
    // Use weak this? Tasks might not stay around
    Thread_Pool::get().submit([shared_this = shared_from_this()](){
        if (!shared_this.is_valid())
        {
            return;
        }

        shared_this->_job();
        //shared_this->_has_executed = true;
        
        for (Weak_Ptr<Task> weak_referencer : shared_this->_references)
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

            shared_referencer->execute();
        } 
    });
}

void Task::reset()
{
    _unfinished_dependencies = 0;
    _dependencies.clear();
    _references.clear();
    _has_executed = false;
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
    return !has_unfinished_dependencies();// && !has_executed(); 
}

Shared_Ptr<Task> Task_Graph::create_task(std::function<void(void)> task_job)
{
    Shared_Ptr<Task> new_task = Shared_Ptr<Task>::create(task_job);
    _tasks.add(new_task);
    return new_task;
}

void Task_Graph::execute()
{
    for (Shared_Ptr<Task> task : _tasks)
    {
        if (!task.is_valid() || !task->can_execute())
        {
            continue;
        }

        task->execute();
    }
}

void Task_Graph::reset()
{
    for (Shared_Ptr<Task> task : _tasks)
    {
        if (!task.is_valid())
        {
            continue;
        }

        task->reset();
    }
}
