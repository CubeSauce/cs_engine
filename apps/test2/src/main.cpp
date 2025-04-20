#include "cs/cs.hpp"
#include "cs/engine/engine.hpp"
#include "cs/engine/name_id.hpp"
#include "cs/engine/task_system.hpp"
#include "cs/memory/weak_ptr.hpp"
#include "cs/containers/hash_map.hpp"
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
    arr.push_back({.i = i, .f = float(i)});
  }

  arr.capacity();
}

void hash_map_test()
{
  Hash_Map<Name_Id, float> map;

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
    Weak_Ptr<int> wp4 = p4;
    Weak_Ptr<int> wp5 = p5;
    Weak_Ptr<int> wp6 = wp5;
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
  tda.push_back(std::format("[{} - {}][{}us]", r, i, time_ms));
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
  PROFILE_FUNCTION()

  Collider sphere;
  sphere.type = Collider::Sphere;
  
  sphere.shape.sphere.radius = 1.0f;
  sphere.bounds = { vec3(-1.0f), vec3(1.0f) };

  Collider capsule;
  capsule.type = Collider::Capsule;
  capsule.shape.capsule.length = 1.0f;
  capsule.shape.capsule.radius = 1.0f;
  capsule.bounds = { vec3(-1.0f, -1.0f, -1.5f), vec3(1.0f, 1.0f, 1.5f) };

  Shared_Ptr<Mesh_Resource> ch_mesh = Shared_Ptr<Mesh_Resource>::create("assets/mesh/ch_cube.obj");
  assert(ch_mesh->submeshes[0].vertices.size() < CONVEX_HULL_MAX_NUM_VERTICES);

  Collider convex;
  convex.type = Collider::Convex_Hull;
  convex.shape.convex_hull.count = (uint32) ch_mesh->submeshes[0].vertices.size();
  int32 count = 0;
  for (const Vertex_Data& vertex : ch_mesh->submeshes[0].vertices)
  {
    convex.shape.convex_hull.vertices[count++] = vertex.vertex_location;
  }

  bool are_colliding;
  Collision_Result result;

  // Sphere - Sphere
  { // Full overlap
    are_colliding = Collision_Test_Function::sphere_sphere(
      sphere, vec3::zero_vector, quat::zero_quat,
      sphere, vec3(0.0f, 0.0f, 0.0f), quat::zero_quat,
      result);

    assert(are_colliding);
    assert(result.normal.nearly_equal(vec3::up_vector));
    assert(is_nearly_equal(result.penetration, 2.0f));
  }
  { // Half overlap - Y
    are_colliding = Collision_Test_Function::sphere_sphere(
      sphere, vec3::zero_vector, quat::zero_quat,
      sphere, vec3(0.0f, 1.5f, 0.0f), quat::zero_quat,
      result);

    assert(are_colliding);
    assert(result.normal.nearly_equal(vec3::forward_vector));
    assert(is_nearly_equal(result.penetration, 0.5f));
  }
  { // Contact, no overlap - X
    are_colliding = Collision_Test_Function::sphere_sphere(
      sphere, vec3::zero_vector, quat::zero_quat,
      sphere, vec3(2.0f, 0.0f, 0.0f), quat::zero_quat,
      result);

    assert(are_colliding);
    assert(result.normal.nearly_equal(vec3::right_vector));
    assert(is_nearly_equal(result.penetration, 0.0f));
  }
  { // Contact, no overlap - XY
    are_colliding = Collision_Test_Function::sphere_sphere(
      sphere, vec3::zero_vector, quat::zero_quat,
      sphere, vec3(2.0f * sin(45_deg), 2.0f * sin(45_deg), 0.0f), quat::zero_quat,
      result);

    assert(are_colliding);
    assert(result.normal.nearly_equal(vec3(1.0f, 1.0f, 0.0f).normalized()));
    assert(is_nearly_equal(result.penetration, 0.0f));
  }
  
  // Sphere - Capsule
  { // Full overlap
    are_colliding = Collision_Test_Function::sphere_capsule(
      sphere, vec3::zero_vector, quat::zero_quat,
      capsule, vec3(0.0f, 0.0f, 0.0f), quat::zero_quat,
      result);

    assert(are_colliding);
    assert(result.normal.nearly_equal(vec3::up_vector));
    assert(is_nearly_equal(result.penetration, 2.0f));
  }
  { // Half overlap - Z
    are_colliding = Collision_Test_Function::sphere_capsule(
      sphere, vec3::zero_vector, quat::zero_quat,
      capsule, vec3(0.0f, 0.0f, 1.5f), quat::zero_quat,
      result);

    assert(are_colliding);
    assert(result.normal.nearly_equal(vec3::up_vector));
    assert(is_nearly_equal(result.penetration, 1.0f));
  }
  { // Half overlap - X
    are_colliding = Collision_Test_Function::sphere_capsule(
      sphere, vec3::zero_vector, quat::zero_quat,
      capsule, vec3(1.0f, 0.0f, 0.0f), quat::zero_quat,
      result);

    assert(are_colliding);
    assert(result.normal.nearly_equal(vec3::right_vector));
    assert(is_nearly_equal(result.penetration, 1.0f));
  }
  { // Contact, no overlap - Y
    are_colliding = Collision_Test_Function::sphere_capsule(
      sphere, vec3::zero_vector, quat::zero_quat,
      capsule, vec3(0.0f, 2.0f, 0.0f), quat::zero_quat,
      result);

    assert(are_colliding);
    assert(result.normal.nearly_equal(vec3::forward_vector));
    assert(is_nearly_equal(result.penetration, 0.0f));
  }
  { // Contact, no overlap - Z
    are_colliding = Collision_Test_Function::sphere_capsule(
      sphere, vec3::zero_vector, quat::zero_quat,
      capsule, vec3(0.0f, 0.0f, -2.5f), quat::zero_quat,
      result);

    assert(are_colliding);
    assert(result.normal.nearly_equal(-vec3::up_vector));
    assert(is_nearly_equal(result.penetration, 0.0f));
  }
  { // Contact, no overlap - YZ
    are_colliding = Collision_Test_Function::sphere_capsule(
      sphere, vec3::zero_vector, quat::zero_quat,
      capsule, vec3(0.0f, 2.0f * sin(45_deg), 0.5f + 2.0f * cos(45_deg)), quat::zero_quat,
      result);

    assert(are_colliding);
    assert(result.normal.nearly_equal(vec3(0.0f, 1.0f, 1.0f).normalized()));
    assert(is_nearly_equal(result.penetration, 0.0f));
  }

  // Capsule - Capsule
  { // Full overlap
    are_colliding = Collision_Test_Function::capsule_capsule(
      capsule, vec3::zero_vector, quat::zero_quat,
      capsule, vec3(0.0f, 0.0f, 0.0f), quat::zero_quat,
      result);

    assert(are_colliding);
    assert(result.normal.nearly_equal(vec3::up_vector));
    assert(is_nearly_equal(result.penetration, 2.0f));
  }
  { // Half overlap - Z
    are_colliding = Collision_Test_Function::capsule_capsule(
      capsule, vec3::zero_vector, quat::zero_quat,
      capsule, vec3(0.0f, 0.0f, 1.5f), quat::zero_quat,
      result);

    assert(are_colliding);
    assert(result.normal.nearly_equal(vec3::up_vector));
    assert(is_nearly_equal(result.penetration, 1.5f));
  }
  { // Half overlap - X
    are_colliding = Collision_Test_Function::capsule_capsule(
      capsule, vec3::zero_vector, quat::zero_quat,
      capsule, vec3(1.0f, 0.0f, 0.0f), quat::zero_quat,
      result);

    assert(are_colliding);
    assert(result.normal.nearly_equal(vec3::right_vector));
    assert(is_nearly_equal(result.penetration, 1.0f));
  }
  { // Contact, no overlap - Y
    are_colliding = Collision_Test_Function::capsule_capsule(
      capsule, vec3::zero_vector, quat::zero_quat,
      capsule, vec3(0.0f, 2.0f, 0.0f), quat::zero_quat,
      result);

    assert(are_colliding);
    assert(result.normal.nearly_equal(vec3::forward_vector));
    assert(is_nearly_equal(result.penetration, 0.0f));
  }
  { // Contact, no overlap - Z
    are_colliding = Collision_Test_Function::capsule_capsule(
      capsule, vec3::zero_vector, quat::zero_quat,
      capsule, vec3(0.0f, 0.0f, -3.0f), quat::zero_quat,
      result);

    assert(are_colliding);
    assert(result.normal.nearly_equal(-vec3::up_vector));
    assert(is_nearly_equal(result.penetration, 0.0f));
  }
  { // Contact, no overlap - YZ
    are_colliding = Collision_Test_Function::capsule_capsule(
      capsule, vec3::zero_vector, quat::zero_quat,
      capsule, vec3(0.0f, 2.0f * sin(45_deg), 1.0f + 2.0f * cos(45_deg)), quat::zero_quat,
      result);

    assert(are_colliding);
    assert(result.normal.nearly_equal(vec3(0.0f, 1.0f, 1.0f).normalized()));
    assert(is_nearly_equal(result.penetration, 0.0f));
  }

  // Capsule - Sphere
  { // Full overlap
    are_colliding = Collision_Test_Function::capsule_sphere(
      capsule, vec3::zero_vector, quat::zero_quat,
      sphere, vec3(0.0f, 0.0f, 0.0f), quat::zero_quat,
      result);

    assert(are_colliding);
    assert(result.normal.nearly_equal(vec3::up_vector));
    assert(is_nearly_equal(result.penetration, 2.0f));
  }
  { // Half overlap - Z
    are_colliding = Collision_Test_Function::capsule_sphere(
      capsule, vec3::zero_vector, quat::zero_quat,
      sphere, vec3(0.0f, 0.0f, 1.5f), quat::zero_quat,
      result);

    assert(are_colliding);
    assert(result.normal.nearly_equal(-vec3::up_vector));
    assert(is_nearly_equal(result.penetration, 1.0f));
  }
  { // Half overlap - X
    are_colliding = Collision_Test_Function::capsule_sphere(
      capsule, vec3::zero_vector, quat::zero_quat,
      sphere, vec3(1.0f, 0.0f, 0.0f), quat::zero_quat,
      result);

    assert(are_colliding);
    assert(result.normal.nearly_equal(-vec3::right_vector));
    assert(is_nearly_equal(result.penetration, 1.0f));
  }
  { // Contact, no overlap - Y
    are_colliding = Collision_Test_Function::capsule_sphere(
      capsule, vec3::zero_vector, quat::zero_quat,
      sphere, vec3(0.0f, 2.0f, 0.0f), quat::zero_quat,
      result);

    assert(are_colliding);
    assert(result.normal.nearly_equal(-vec3::forward_vector));
    assert(is_nearly_equal(result.penetration, 0.0f));
  }
  { // Contact, no overlap - Z
    are_colliding = Collision_Test_Function::capsule_sphere(
      capsule, vec3::zero_vector, quat::zero_quat,
      sphere, vec3(0.0f, 0.0f, -2.5f), quat::zero_quat,
      result);

    assert(are_colliding);
    assert(result.normal.nearly_equal(vec3::up_vector));
    assert(is_nearly_equal(result.penetration, 0.0f));
  }
  { // Contact, no overlap - YZ
    are_colliding = Collision_Test_Function::capsule_sphere(
      capsule, vec3::zero_vector, quat::zero_quat,
      sphere, vec3(0.0f, 2.0f * sin(45_deg), 0.5f + 2.0f * cos(45_deg)), quat::zero_quat,
      result);

    assert(are_colliding);
    assert(result.normal.nearly_equal(-vec3(0.0f, 1.0f, 1.0f).normalized()));
    assert(is_nearly_equal(result.penetration, 0.0f));
  }

  // Convex - Convex
  { // Full overlap
    are_colliding = Collision_Test_Function::convex_convex(
      convex, vec3::zero_vector, quat::zero_quat,
      convex, vec3(0.0f, 0.0f, 0.0f), quat::zero_quat,
      result);

    assert(are_colliding);
    //assert(result.normal.nearly_equal(vec3::up_vector));
    assert(is_nearly_equal(result.penetration, 2.0f));
  }
  { // Half overlap
    are_colliding = Collision_Test_Function::convex_convex(
      convex, vec3::zero_vector, quat::zero_quat,
      convex, vec3(0.0f, 0.0f, 1.0f), quat::zero_quat,
      result);

    assert(are_colliding);
    assert(result.normal.nearly_equal(vec3::up_vector));
    assert(is_nearly_equal(result.penetration, 1.0f));
  }
  { // Contact. overlap
    are_colliding = Collision_Test_Function::convex_convex(
      convex, vec3::zero_vector, quat::zero_quat,
      convex, vec3(0.0f, 0.0f, 2.0f), quat::zero_quat,
      result);

    assert(are_colliding);
    assert(result.normal.nearly_equal(vec3::up_vector));
    assert(is_nearly_equal(result.penetration, 0.0f));
  }
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

  Profiler::get().write_to_chrometracing_json("tests.json");
}

int main(int argc, char** argv)
{
  engine_test({"cs_num_threads=1", "cs_vr_support=0"});

  return 0;
}