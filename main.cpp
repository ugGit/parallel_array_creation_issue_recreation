#include "CountingIterator.h"

#include <stdio.h>
#include <algorithm>
#include <execution>

constexpr int N = 10;
constexpr int M = 1;

struct obj{
  int x = 1;
  int y = 2;
};

int main(){
  // reserve space for M pointers
  obj** nested_arrays = new obj*[M];

  // set a placeholder to track the address where nested_arrays[x] is pointing to, this will be modified later
  nested_arrays[0] = new obj[N];
  printf("Pointed to addresses after initialization outside the loop\n");
  printf("1a) &(nested_arrays[0][1] = %p\n", &(nested_arrays[0][1]));
  printf("1b) &(nested_arrays[0][2] = %p\n", &(nested_arrays[0][2]));

  printf("--------\n");
  printf("Initial value acces verification: \n");
  printf("before) nested_arrays[0][1] = %d\n", nested_arrays[0][1].x);
  nested_arrays[0][1].x = 99;
  printf("after)  nested_arrays[0][1] = %d\n", nested_arrays[0][1].x);
  printf("--------\n");

  // create in parallel arrays on the heap and store the corresponding pointer in the nested_array
  std::for_each_n(std::execution::par, counting_iterator(0), M, [=](unsigned int i){
    obj* obj_collection = new obj[N];
    printf("Reassign the pointer within the parallel loop\n");
    printf("2a) &(nested_arrays[0][1] = %p\n", &(nested_arrays[0][1]));
    nested_arrays[i] = obj_collection;
    printf("3a) &(nested_arrays[0][1] = %p\n", &(nested_arrays[0][1]));
  });
  // verify adresses and access
  printf("--------\n");
  printf("Pointed to addresses after reassignation outside the loop\n");
  printf("4a) &(nested_arrays[0][1] = %p\n", &(nested_arrays[0][1]));
  printf("4b) &(nested_arrays[0][2] = %p\n", &(nested_arrays[0][2]));
  printf("--------\n");
  printf("Final value acces verification: \n");
  printf("before) nested_arrays[0][1] = %d\n", nested_arrays[0][1].x);
  nested_arrays[0][1].x = 66;
  printf("after)  nested_arrays[0][1] = %d\n", nested_arrays[0][1].x);

  return 0;
}
