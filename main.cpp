#include "gc_pointer.h"
#include "LeakTester.h"
#include <assert.h>


int main()
{
    Pointer<int> p = new int(19);
 	assert(*p == 19);
  
    p = new int(21);
  	assert(*p == 21);
  
    p = new int(28);
 	assert(*p == 28);
  
  	Pointer<int> q(new int(13));
  	assert(*q == 13);
  	p = q;
  	
  	assert(p == q);
  	assert(*p == *q);
  
  	Pointer<int> r;
  	p = r;
  	assert(!(int*)p);
  
    r = q;
  	assert(*r == 13);
  
  	r = nullptr;
  	assert(!(int*)r);
  
  	r = new int(129);
  	assert((int*)r);
  	assert(*r == 129);
  
  	r = new int(225);
  	assert((int*)r);
  	assert(*r == 225);
  
  	Pointer<unsigned > x;
    Pointer<unsigned>::showlist();
  	//q = x;
//   	int *n = nullptr;
//   	assert((int *)x == n);
  
  	//int *foo = x;
  	printf("x: %p\n", (unsigned *)x);
  	assert((unsigned*)x == nullptr);
  	
  	Pointer<unsigned> z(new unsigned(33u));
  	assert((unsigned*)z);
    assert(*z == 33u);
  	z = x;
  	assert(!(unsigned*)z);
  

  	Pointer<unsigned, 13> mySizeTArray;
  	assert(! ((unsigned *)mySizeTArray));
  	
  	mySizeTArray = new unsigned[13]
    {1,2,3,4,5,6,7,8,9,10,11,12,13};
  	
  	assert( ((unsigned *)mySizeTArray));
  
  	Pointer<unsigned>::GCiterator current = mySizeTArray.begin();
 	//assert(current.size() == 13 * sizeof(unsigned));
  	printf("iter size: %u\n", current.size());
  	for(Pointer<unsigned>::GCiterator end = mySizeTArray.end();
        current != end;
 		current++)
    {
      printf("Array value using iter: %u\n", (*current));
    }
  
  	for(int i = 0; i < 13; i++)
    {
      printf("Array value using indexer of Pointer[%d]: %u\n", i, mySizeTArray[i]);
    }
   
  	current = mySizeTArray.begin();
  	for(int i = 0; i < 13; i++)
    {
      printf("Array value using indexer of Iter[%d]: %u\n", i, current[i]);
    }	
  
    return 0;
}