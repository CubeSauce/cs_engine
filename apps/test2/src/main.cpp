#include "cs/cs.hpp"
#include "cs/engine/engine.hpp"
#include "cs/engine/name_id.hpp"
#include "cs/engine/task_system.hpp"
#include "cs/memory/weak_ptr.hpp"
#include "cs/containers/hash_table.hpp"

#include <chrono>
#include <shared_mutex>

struct Test
{
  int i;
  float f;
};

void dynamic_array_test()
{
  Dynamic_Array<Test> arr;

  for (int i = 0; i < 1024 * 4; ++i)
  {
    arr.add({.i = i, .f = float(i)});
  }

  arr.capacity();
}

void hash_map_test()
{
  Hash_Map<float> map;

  map.add(Name_Id("one"), 1.0f);
  map.add(Name_Id("two"), 1.0f);
  map.add(Name_Id("two"), 2.0f);
  map.add(Name_Id("three"), 3.0f);

  float *one = map.find(Name_Id("one"));
  float *two = map.find(Name_Id("two"));
  float *three = map.find(Name_Id("three"));
  float& four = map.find_or_add(Name_Id("four"));
  map.add(Name_Id("four"), 4.0f);
  float *four_p = map.find(Name_Id("four"));
  float *five = map.find(Name_Id("five"));
}

void smart_ptr_test()
{
  Shared_Ptr<int> p1 = Shared_Ptr<int>::create();
  Weak_Ptr<int> wp1(p1);
  Shared_Ptr<int> p2 = wp1.lock();
  p1.release();
  Weak_Ptr<int> wp2 = wp1;
  wp1 = wp2;
  wp1.release();

  {
    Shared_Ptr<int> p3 = wp2.lock();
    Shared_Ptr<int> p4 = p3;
    Shared_Ptr<int> p5 = p4;
    Weak_Ptr<int> wp3 = p5;
    Weak_Ptr<int> wp4 = wp4;
    Weak_Ptr<int> wp5 = wp5;
    Weak_Ptr<int> wp6 = wp6;
  }

  assert(p2.is_valid());
  p2.release();
  assert(!wp2.is_valid());

}

std::chrono::steady_clock::time_point start;
Shared_Ptr<Task> task;

std::shared_mutex mutex;
Dynamic_Array<std::string> tda;

void task_worker(int r, int i)
{
  std::unique_lock lock(mutex);
  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
  int64 time_ms = std::chrono::duration_cast<std::chrono::microseconds>(now - start).count();
  tda.add(std::format("[{} - {}][{}us]", r, i, time_ms));
  start = std::chrono::steady_clock::now();
  lock.unlock();
  
  //using namespace std::chrono_literals;
  //std::this_thread::sleep_for(1ms);
}

void task_test(int32 num_reps, int32 num_tasks)
{
  start = std::chrono::steady_clock::now();

  for (int32 r = 0; r < num_reps; r++)
  {
    Task_Graph task_graph;
    for (int32 i = 0; i < num_tasks; ++i)
    {
      task = task_graph.create_task(std::bind(task_worker, r, i));
    }

    task_graph.execute();
  }
  
  for (int i = 0; i < tda.size(); ++i)
  {
    if (i%100 == 0)
    {
      printf("%s\n", tda[i].c_str());
    }
  }

  task.release();
}

void engine_test(const Dynamic_Array<std::string>& args)
{
  Engine engine;
  engine.initialize(args);

  dynamic_array_test();
  hash_map_test();
  smart_ptr_test();
  task_test(1000, 120);

  engine.shutdown();
}

int main(int argc, char** argv)
{
  engine_test({"cs_num_threads=1", "cs_vr_support=0"});

  return 0;
}