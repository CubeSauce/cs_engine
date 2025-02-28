#include "cs/cs.hpp"
#include "cs/engine/name_id.hpp"
#include "cs/memory/weak_ptr.hpp"
#include "cs/containers/hash_table.hpp"

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

int main(int argc, char** argv)
{
  hash_map_test();
  smart_ptr_test();

  return 0;
}