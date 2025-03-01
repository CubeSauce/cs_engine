#include "cs/cs.hpp"
#include "cs/engine/engine.hpp"
#include "cs/engine/name_id.hpp"
#include "cs/engine/task_system.hpp"
#include "cs/memory/weak_ptr.hpp"
#include "cs/containers/hash_table.hpp"

#include <chrono>

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
  assert(p2.is_valid());
  p2.release();
  assert(!wp2.is_valid());
}

std::chrono::steady_clock::time_point start;
Shared_Ptr<Task> task_physics, task_animation, task_render;

Dynamic_Array<std::string> qp, qa, qr;

void task_test(int32 num_iterations)
{
  std::mutex m;

  start = std::chrono::steady_clock::now();
  for (int32 i = 0; i < num_iterations; ++i)
  {
    Task_Graph task_graph;
  
    task_physics = task_graph.create_task([](){
      std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
      int64 time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
  
      qp.add(std::format("[{}ms] Physics", time_ms));
  
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(1ms);
    });
    
    task_animation = task_graph.create_task([](){
      std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
      int64 time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
  
      qa.add(std::format("[{}ms] Animation", time_ms));
  
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(2ms);
    });
    
    task_render = task_graph.create_task([](){
      std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
      int64 time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
  
      qr.add(std::format("[{}ms] Render", time_ms));
  
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(4ms);
    });
  
    task_render->add_dependency(task_physics);
    task_render->add_dependency(task_animation);

    task_graph.execute();

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(16ms);
  }

  using namespace std::chrono_literals;
  std::this_thread::sleep_for(500ms);
  for (int i = 0; i < qp.size(); ++i)
  {

    printf("%s\n%s\n%s\n", qp[i].c_str(), qa[i].c_str(), qr[i].c_str());
  }

  task_physics.release();
  task_animation.release();
  task_render.release();
}

void engine_test(const Dynamic_Array<std::string>& args)
{
  Engine engine;
  engine.initialize(args);

  hash_map_test();
  smart_ptr_test();
  task_test(3);

  engine.shutdown();
}

int main(int argc, char** argv)
{
  engine_test({"cs_num_threads=5", "cs_vr_support=0"});

  return 0;
}