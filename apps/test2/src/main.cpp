#include "cs/cs.hpp"
#include "cs/engine/engine.hpp"
#include "cs/engine/name_id.hpp"
#include "cs/engine/task_system.hpp"
#include "cs/memory/weak_ptr.hpp"
#include "cs/containers/hash_table.hpp"
#include "cs/engine/physics/physics_system.hpp"
#include "cs/engine/physics/collision_function.hpp"

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

void collision_test()
{
  vec3 result_normal;
  float result_penetration;
  bool are_colliding;

  Collider sphere;
  sphere.type = Collider::Sphere;
  sphere.shape.sphere.radius = 1.0f;

  Collider capsule;
  capsule.type = Collider::Capsule;
  capsule.shape.capsule.length = 1.0f;
  capsule.shape.capsule.radius = 1.0f;

  // Sphere - Sphere
  { // Full overlap
    are_colliding = Collision_Test_Function::sphere_sphere(
      sphere, vec3::zero_vector, quat::zero_quat,
      sphere, vec3(0.0f, 0.0f, 0.0f), quat::zero_quat,
      result_normal, result_penetration);

    assert(are_colliding);
    assert(result_normal.nearly_equal(vec3::up_vector));
    assert(is_nearly_equal(result_penetration, 2.0f));
  }
  { // Half overlap - Y
    are_colliding = Collision_Test_Function::sphere_sphere(
      sphere, vec3::zero_vector, quat::zero_quat,
      sphere, vec3(0.0f, 1.5f, 0.0f), quat::zero_quat,
      result_normal, result_penetration);

    assert(are_colliding);
    assert(result_normal.nearly_equal(vec3::forward_vector));
    assert(is_nearly_equal(result_penetration, 0.5f));
  }
  { // Contact, no overlap - X
    are_colliding = Collision_Test_Function::sphere_sphere(
      sphere, vec3::zero_vector, quat::zero_quat,
      sphere, vec3(2.0f, 0.0f, 0.0f), quat::zero_quat,
      result_normal, result_penetration);

    assert(are_colliding);
    assert(result_normal.nearly_equal(vec3::right_vector));
    assert(is_nearly_equal(result_penetration, 0.0f));
  }
  { // Contact, no overlap - XY
    are_colliding = Collision_Test_Function::sphere_sphere(
      sphere, vec3::zero_vector, quat::zero_quat,
      sphere, vec3(2.0f * sin(MATH_DEG_TO_RAD(45.0f)), 2.0f * sin(MATH_DEG_TO_RAD(45.0f)), 0.0f), quat::zero_quat,
      result_normal, result_penetration);

    assert(are_colliding);
    assert(result_normal.nearly_equal(vec3(1.0f, 1.0f, 0.0f).normalized()));
    assert(is_nearly_equal(result_penetration, 0.0f));
  }
  
  // Sphere - Capsule
  { // Full overlap
    are_colliding = Collision_Test_Function::sphere_capsule(
      sphere, vec3::zero_vector, quat::zero_quat,
      capsule, vec3(0.0f, 0.0f, 0.0f), quat::zero_quat,
      result_normal, result_penetration);

    assert(are_colliding);
    assert(result_normal.nearly_equal(vec3::up_vector));
    assert(is_nearly_equal(result_penetration, 2.0f));
  }
  { // Half overlap - Z
    are_colliding = Collision_Test_Function::sphere_capsule(
      sphere, vec3::zero_vector, quat::zero_quat,
      capsule, vec3(0.0f, 0.0f, 1.5f), quat::zero_quat,
      result_normal, result_penetration);

    assert(are_colliding);
    assert(result_normal.nearly_equal(vec3::up_vector));
    assert(is_nearly_equal(result_penetration, 1.0f));
  }
  { // Half overlap - X
    are_colliding = Collision_Test_Function::sphere_capsule(
      sphere, vec3::zero_vector, quat::zero_quat,
      capsule, vec3(1.0f, 0.0f, 0.0f), quat::zero_quat,
      result_normal, result_penetration);

    assert(are_colliding);
    assert(result_normal.nearly_equal(vec3::right_vector));
    assert(is_nearly_equal(result_penetration, 1.0f));
  }
  { // Contact, no overlap - Y
    are_colliding = Collision_Test_Function::sphere_capsule(
      sphere, vec3::zero_vector, quat::zero_quat,
      capsule, vec3(0.0f, 2.0f, 0.0f), quat::zero_quat,
      result_normal, result_penetration);

    assert(are_colliding);
    assert(result_normal.nearly_equal(vec3::forward_vector));
    assert(is_nearly_equal(result_penetration, 0.0f));
  }
  { // Contact, no overlap - Z
    are_colliding = Collision_Test_Function::sphere_capsule(
      sphere, vec3::zero_vector, quat::zero_quat,
      capsule, vec3(0.0f, 0.0f, -2.5f), quat::zero_quat,
      result_normal, result_penetration);

    assert(are_colliding);
    assert(result_normal.nearly_equal(-vec3::up_vector));
    assert(is_nearly_equal(result_penetration, 0.0f));
  }
  { // Contact, no overlap - YZ
    are_colliding = Collision_Test_Function::sphere_capsule(
      sphere, vec3::zero_vector, quat::zero_quat,
      capsule, vec3(0.0f, 2.0f * sin(MATH_DEG_TO_RAD(45.0f)), 0.5f + 2.0f * cos(MATH_DEG_TO_RAD(45.0f))), quat::zero_quat,
      result_normal, result_penetration);

    assert(are_colliding);
    assert(result_normal.nearly_equal(vec3(0.0f, 1.0f, 1.0f).normalized()));
    assert(is_nearly_equal(result_penetration, 0.0f));
  }

  // Capsule - Capsule
  { // Full overlap
    are_colliding = Collision_Test_Function::capsule_capsule(
      capsule, vec3::zero_vector, quat::zero_quat,
      capsule, vec3(0.0f, 0.0f, 0.0f), quat::zero_quat,
      result_normal, result_penetration);

    assert(are_colliding);
    assert(result_normal.nearly_equal(vec3::up_vector));
    assert(is_nearly_equal(result_penetration, 2.0f));
  }
  { // Half overlap - Z
    are_colliding = Collision_Test_Function::capsule_capsule(
      capsule, vec3::zero_vector, quat::zero_quat,
      capsule, vec3(0.0f, 0.0f, 1.5f), quat::zero_quat,
      result_normal, result_penetration);

    assert(are_colliding);
    assert(result_normal.nearly_equal(vec3::up_vector));
    assert(is_nearly_equal(result_penetration, 1.5f));
  }
  { // Half overlap - X
    are_colliding = Collision_Test_Function::capsule_capsule(
      capsule, vec3::zero_vector, quat::zero_quat,
      capsule, vec3(1.0f, 0.0f, 0.0f), quat::zero_quat,
      result_normal, result_penetration);

    assert(are_colliding);
    assert(result_normal.nearly_equal(vec3::right_vector));
    assert(is_nearly_equal(result_penetration, 1.0f));
  }
  { // Contact, no overlap - Y
    are_colliding = Collision_Test_Function::capsule_capsule(
      capsule, vec3::zero_vector, quat::zero_quat,
      capsule, vec3(0.0f, 2.0f, 0.0f), quat::zero_quat,
      result_normal, result_penetration);

    assert(are_colliding);
    assert(result_normal.nearly_equal(vec3::forward_vector));
    assert(is_nearly_equal(result_penetration, 0.0f));
  }
  { // Contact, no overlap - Z
    are_colliding = Collision_Test_Function::capsule_capsule(
      capsule, vec3::zero_vector, quat::zero_quat,
      capsule, vec3(0.0f, 0.0f, -3.0f), quat::zero_quat,
      result_normal, result_penetration);

    assert(are_colliding);
    assert(result_normal.nearly_equal(-vec3::up_vector));
    assert(is_nearly_equal(result_penetration, 0.0f));
  }
  { // Contact, no overlap - YZ
    are_colliding = Collision_Test_Function::capsule_capsule(
      capsule, vec3::zero_vector, quat::zero_quat,
      capsule, vec3(0.0f, 2.0f * sin(MATH_DEG_TO_RAD(45.0f)), 1.0f + 2.0f * cos(MATH_DEG_TO_RAD(45.0f))), quat::zero_quat,
      result_normal, result_penetration);

    assert(are_colliding);
    assert(result_normal.nearly_equal(vec3(0.0f, 1.0f, 1.0f).normalized()));
    assert(is_nearly_equal(result_penetration, 0.0f));
  }

  // Capsule - Sphere
  { // Full overlap
    are_colliding = Collision_Test_Function::capsule_sphere(
      capsule, vec3::zero_vector, quat::zero_quat,
      sphere, vec3(0.0f, 0.0f, 0.0f), quat::zero_quat,
      result_normal, result_penetration);

    assert(are_colliding);
    assert(result_normal.nearly_equal(vec3::up_vector));
    assert(is_nearly_equal(result_penetration, 2.0f));
  }
  { // Half overlap - Z
    are_colliding = Collision_Test_Function::capsule_sphere(
      capsule, vec3::zero_vector, quat::zero_quat,
      sphere, vec3(0.0f, 0.0f, 1.5f), quat::zero_quat,
      result_normal, result_penetration);

    assert(are_colliding);
    assert(result_normal.nearly_equal(-vec3::up_vector));
    assert(is_nearly_equal(result_penetration, 1.0f));
  }
  { // Half overlap - X
    are_colliding = Collision_Test_Function::capsule_sphere(
      capsule, vec3::zero_vector, quat::zero_quat,
      sphere, vec3(1.0f, 0.0f, 0.0f), quat::zero_quat,
      result_normal, result_penetration);

    assert(are_colliding);
    assert(result_normal.nearly_equal(-vec3::right_vector));
    assert(is_nearly_equal(result_penetration, 1.0f));
  }
  { // Contact, no overlap - Y
    are_colliding = Collision_Test_Function::capsule_sphere(
      capsule, vec3::zero_vector, quat::zero_quat,
      sphere, vec3(0.0f, 2.0f, 0.0f), quat::zero_quat,
      result_normal, result_penetration);

    assert(are_colliding);
    assert(result_normal.nearly_equal(-vec3::forward_vector));
    assert(is_nearly_equal(result_penetration, 0.0f));
  }
  { // Contact, no overlap - Z
    are_colliding = Collision_Test_Function::capsule_sphere(
      capsule, vec3::zero_vector, quat::zero_quat,
      sphere, vec3(0.0f, 0.0f, -2.5f), quat::zero_quat,
      result_normal, result_penetration);

    assert(are_colliding);
    assert(result_normal.nearly_equal(vec3::up_vector));
    assert(is_nearly_equal(result_penetration, 0.0f));
  }
  { // Contact, no overlap - YZ
    are_colliding = Collision_Test_Function::capsule_sphere(
      capsule, vec3::zero_vector, quat::zero_quat,
      sphere, vec3(0.0f, 2.0f * sin(MATH_DEG_TO_RAD(45.0f)), 0.5f + 2.0f * cos(MATH_DEG_TO_RAD(45.0f))), quat::zero_quat,
      result_normal, result_penetration);

    assert(are_colliding);
    assert(result_normal.nearly_equal(-vec3(0.0f, 1.0f, 1.0f).normalized()));
    assert(is_nearly_equal(result_penetration, 0.0f));
  }



  //Collision_Test_Function::sphere_capsule();
  //Collision_Test_Function::capsule_sphere();
}

void engine_test(const Dynamic_Array<std::string>& args)
{
  Engine engine;
  engine.initialize(args);

  dynamic_array_test();
  hash_map_test();
  smart_ptr_test();
  //task_test(1000, 120);
  collision_test();

  engine.shutdown();
}

int main(int argc, char** argv)
{
  engine_test({"cs_num_threads=1", "cs_vr_support=0"});

  return 0;
}