#include "cs/cs.hpp"
#include "cs/engine/name_id.hpp"
#include "cs/containers/hash_table.hpp"

int main(int argc, char** argv)
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

  return 0;
}