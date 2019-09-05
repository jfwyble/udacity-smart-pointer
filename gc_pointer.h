#include <iostream>
#include <list>
#include <typeinfo>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include "gc_details.h"
#include "gc_iterator.h"
/*
    Pointer implements a pointer type that uses
    garbage collection to release unused memory.
    A Pointer must only be used to point to memory
    that was dynamically allocated using new.
    (You can point to the stack as long as that memory remains in scope, 
    but using this on such memory would possibly cause a segfault.)
    When used to refer to an allocated array,
    specify the array size.
*/
template <class T, int size = 0>
class Pointer{
private:
    // refContainer maintains the garbage collection list.
    static std::list<PtrDetails<T> > refContainer;
    // addr points to the allocated memory to which
    // this Pointer pointer currently points.
    T *addr;
    /*  isArray is true if this Pointer points
        to an allocated array. It is false
        otherwise. 
    */
    bool isArray; 
    // true if pointing to array
    // If this Pointer is pointing to an allocated
    // array, then arraySize contains its size.
    unsigned arraySize; // size of the array
    static bool first; // true when first Pointer is created
    // Return an iterator to pointer details in refContainer.
    typename std::list<PtrDetails<T> >::iterator findPtrInfo(T *ptr);
public:
    // Define an iterator type for Pointer<T>.
    typedef Iter<T> GCiterator;
    // Empty constructor
    // NOTE: templates aren't able to have prototypes with default arguments
    // this is why constructor is designed like this:
    Pointer() :
    	Pointer(nullptr)
    {

        /*
        This (meaning calling the second overload from inside this cTor,
        instead of calling it per the syntax above,
        was a subtle bug, the likes of which unmanaged languages are famous for.
        It would compile without warnings, yet cause the 'addr' to be set to
        some garbage address when I compared this instance to nullptr, when I constructed an empty Pointer<T>:
        */
        
        // Pointer(NULL);
          
    }
    Pointer(T*);
    // Copy constructor.
    Pointer(const Pointer &);
    // Destructor for Pointer.
    ~Pointer();
    // Collect garbage. Returns true if at least
    // one object was freed.
    static bool collect();
 
    // Overload assignment of pointer to Pointer.
    T *operator=(T *t);
    // Overload assignment of Pointer to Pointer.
    Pointer &operator=(Pointer &rv);
    // Return a reference to the object pointed
    // to by this Pointer.
    T &operator*(){
      	if(addr)
        	return *addr;
      	else
          throw std::runtime_error(
            std::string(__PRETTY_FUNCTION__).append(": Unable to dereference an invalid pointer. Assign this object to a valid pointer before dereferencing."));
    }
    // Return the address being pointed to.
    T *operator->() { return addr; }
    // Return a reference to the object at the
    // index specified by i.
    T &operator[](int i){ return addr[i];}
    // Conversion function to T *.
    operator T *() { printf("%s returning %p\n", __PRETTY_FUNCTION__, addr); return addr; }
  	// Logical equality operator for smart pointer
  	bool operator==(const Pointer<T,size> &other)
    {
      if(!this->isArray && !other.isArray)
      {
        return this->addr == other.addr;
      }
      else if(this->isArray && other.isArray)
      {
        // It is difficult to reinforce anything with 
        // callers who could possibly point to different ranges in the same array:
        //
        return (this->addr == other.addr) && (this->arraySize == other.arraySize);
      }
      else
      {
        return false;
      }
    }
 	// Logical equality operator for naked pointer:
  	bool operator==(T *ptr)
    {
      // This gets a little involved, becuase if I'm an array, and someone passes in the address
      // of one of my elements past start, I need to return true.
      if(this->isArray)
      {
        return this->addr == ptr;
      }
      else
      {
        if(this->arraySize == 1u)
        {
          return this->addr == ptr;
        }
        else
        {
          // We can use math, instead of iterating with an Iter
          //
          intptr_t maxAddress = (intptr_t)addr + (sizeof(T) * arraySize); 
          
          return ((intptr_t)ptr >= (intptr_t)addr && (intptr_t)ptr <= maxAddress);
        }
      }
    }
    // Return an Iter to the start of the allocated memory.
    Iter<T> begin(){
        int _size;
        if (isArray)
            _size = arraySize;
        else
            _size = 1;
        return Iter<T>(addr, addr, addr + _size);
    }
    // Return an Iter to one past the end of an allocated array.
    Iter<T> end(){
        int _size;
        if (isArray)
            _size = arraySize;
        else
            _size = 1;
        return Iter<T>(addr + _size, addr, addr + _size);
    }
    // Return the size of refContainer for this type of Pointer.
    static int refContainerSize() { return refContainer.size(); }
    // A utility function that displays refContainer.
    static void showlist();
    // Clear refContainer when program exits.
    static void shutdown();
private:
    // Both the destructor and the assignment operators need the same logic to decrement ref count and call collect:
  	void cleanup();
  
  	// Upsert pointer details into the list
  	static bool registerPointer(T* ptr);
};

// STATIC INITIALIZATION
// Creates storage for the static variables
template <class T, int size>
std::list<PtrDetails<T> > Pointer<T, size>::refContainer;
template <class T, int size>
bool Pointer<T, size>::first = true;

// Constructor for both initialized and uninitialized objects. -> see class interface
template<class T,int size>
Pointer<T,size>::Pointer(T *t) :
	addr(t),
	arraySize(size)
{
    // Register shutdown() as an exit function.
    if (first)
        atexit(shutdown);
    first = false;

    // DONE?: Implement Pointer constructor
    // Lab: Smart Pointer Project Lab
    // If we point to something, and not nothing,
    // upsert a PtrDetails<T> in the static list
    // and increment refcount if update
      
    this->isArray = (size > 0);
      
    if(addr) registerPointer(addr);
    
    printf("%s: addr: %p\n", __PRETTY_FUNCTION__, addr);
}
// Copy constructor.
template< class T, int size>
Pointer<T,size>::Pointer(const Pointer &ob){

    // DONE?: Implement Pointer constructor
    // Lab: Smart Pointer Project Lab
    puts(__PRETTY_FUNCTION__);
    /*
    1) Copy the details ("point" to the same address)
    2) Find the details object in the static list and update the reference count:
    */	
    this->addr = ob.addr;
    this->isArray = ob.isArray;
    
    auto found = findPtrInfo(addr),
       end = refContainer.end();
    if(found != end)
    {
    	PtrDetails<T> info = (*found);
        // Increment the refcount since now there is one more pointing to this address:
        //
        info.refcount++;
    }
    
}

// Common cleanup logic for both destructor and assignemnt operators that can set a valid instance to nullptr:
template <class T, int size>
void Pointer<T, size>::cleanup()
{
    // If we point at NULL, just chill!!
    // We are uninitialized!
    // Otherwise, find the info object in the static list:
    //
   if(addr)
    {
      // Find the gc_details for the pointer address we hold:
      //
      auto found = findPtrInfo(addr),
      	end = refContainer.end();
        
      if(found != end)
      {
      	PtrDetails<T> info = (*found);
      	info.refcount--;
      }
      else
      {
      	// I would throw an exception here, except that this might be a destructor:
        //
        printf("%s: ERROR: Unable to find PtrDetails object for pointer address %p.", __PRETTY_FUNCTION__, addr);
      }      
      collect();
    }
}

// Destructor for Pointer.
template <class T, int size>
Pointer<T, size>::~Pointer(){
    
    // DONE?: Implement Pointer destructor
    // Lab: New and Delete Project Lab
    
   printf("%s: %p\n", __PRETTY_FUNCTION__, addr);
    
   cleanup();   
}

// Upsert to the list. Returns true if added,
// false if updated
template <class T, int size>
bool Pointer<T, size>::registerPointer(T *ptr)
{
  bool wasInserted = false;
  	
  // Return an iterator to pointer details in refContainer.
  
  	auto found = std::find_if(refContainer.begin(),
                           refContainer.end(),
              [&](const PtrDetails<T> &ptrDetails)->bool
              {
                return ptrDetails.memPtr == ptr;
              }
    ),
  	end = refContainer.end();
  
  
  	if(found != end)
  	{
      (*found).refcount++;
    }
  	else
    {
      wasInserted = true;
      
      // The cTor sets array attributes using the size argument
      // and initializes refcount to 1u:
      //
      //PtrDetails<T> details(ptr, size);
      refContainer.emplace_back(ptr, size);
    }
  	
  	return wasInserted;
}

// Collect garbage. Returns true if at least
// one object was freed.
template <class T, int size>
bool Pointer<T, size>::collect(){

    // DONE?: Implement collect function
    // LAB: New and Delete Project Lab
    // Note: collect() will be called in the destructor
	std::vector<typename std::list<PtrDetails<T> >::iterator> pointersToRemove;
    bool wasMemoryDeallocated = false;
    // Two things need to be accomplished here.
    // Firstly, deallocate memory for any smart pointer whose refcount has reached zero:
	// 
   
   for(typename std::list<PtrDetails<T> >::iterator current = refContainer.begin(), end = refContainer.end();
   			current != end;
            current++
   )
   {
   		auto &ptrDetails = (*current);
     	if(ptrDetails.refcount == 0u)
          {
              pointersToRemove.push_back(current);
              if(ptrDetails.isArray)
              {
                  delete [] ptrDetails.memPtr;
              }
              else
              {
                  delete ptrDetails.memPtr;
              }
              ptrDetails.memPtr = nullptr;
              wasMemoryDeallocated = true;
          } 
   }
    
    // Secondly (and less obviously), remove the PtrDetails objects from the list
    // after the deallocations are completed:
    //
    for(auto & iter : pointersToRemove)
    {
    	 refContainer.erase(iter);
    }
    
    return wasMemoryDeallocated;
}

// Overload assignment of pointer to Pointer.
template <class T, int size>
T *Pointer<T, size>::operator=(T *t){

    // TODO: Implement operator==
    // LAB: Smart Pointer Project Lab
    
    /*
    Use cases:
    1) this is null, t is null
    2) this is null, t is non-null
    3) this is non-null, t is null
    4) this is non-null, t is non-null
    	a) equal
        b) unequal
    */
  	if(!addr) // this is null
    {
    	if(!t) // t is null
        	return nullptr;
        else // t is non-null
        {
        	// 1 upsert pointer details
            registerPointer(t);
            // 2 update ptr field and return address
            return (this->addr = t);
        }
    }
   else // this is non-null
   {
   		if (!t) // t is null
        {
        	// 1 find ptr details and decrement refcount, and collect
            cleanup();
            // 2 set addr null and return nullptr
            return (this->addr = nullptr);        
         }
         else
         {
         	// 1 find ptr details and decrement refcount, and collect
         	cleanup();
            // 2 upsert new pointer
            registerPointer(t);
            // 3 set new addr and return
            return (this->addr = t);
         }
   }

}
// Overload assignment of Pointer to Pointer.
template <class T, int size>
Pointer<T, size> &Pointer<T, size>::operator=(Pointer &rv){

    // TODO: Implement operator==
    // LAB: Smart Pointer Project Lab
    
      /*
    Use cases:
    1) this is null, and it is assigned to null (easy)
    2) this is null, and it is assigned to non-null (happy)
    3) this is non-null, and it is assigned to null (angry)
    4) this is non-null, and it is assigned to non-null logically equal
    5) this is non-null, and it is assigned to non-null NOT logically equal
    */
    
    if(!addr) // (1,2) this is null
    {
    	if(rv.addr) // 2) this is null, and it is assigned to non-null (happy)
        {
        	registerPointer(rv.addr);
            this->addr = rv.addr;
        }
        // 1) this is null, and it is assigned to null (easy) 
    }
	else // (3,4,5) this is non-null
    {
    	if(rv.addr) // 4,5 rv is non-null
        {
        	// 5) this is non-null, and it is assigned to non-null NOT logically equal
            if(this->addr != rv.addr)
            {
            	cleanup(); // let go of our current pointer
                this->addr = rv.addr;   
             }
       		// 4) this is non-null, and it is assigned to non-null logically equal
         	// In both cases, the pointer passed in needs to be upserted into the list:
            registerPointer(rv.addr);
        }
        else
        {
        	// 3 rv is null
            cleanup();
            this->addr = nullptr;
        }
    }
    return *this;
}

// A utility function that displays refContainer.
template <class T, int size>
void Pointer<T, size>::showlist(){
    typename std::list<PtrDetails<T> >::iterator p;
    std::cout << "refContainer<" << typeid(T).name() << ", " << size << ">:\n";
    std::cout << "memPtr refcount value\n ";
    if (refContainer.begin() == refContainer.end())
    {
        std::cout << " Container is empty!\n\n ";
    }
    for (p = refContainer.begin(); p != refContainer.end(); p++)
    {
        std::cout << "[" << (void *)p->memPtr << "]"
             << " " << p->refcount << " ";
        if (p->memPtr)
            std::cout << " " << *p->memPtr;
        else
            std::cout << "---";
        std::cout << std::endl;
    }
    std::cout << std::endl;
}
// Find a pointer in refContainer.
template <class T, int size>
typename std::list<PtrDetails<T> >::iterator
Pointer<T, size>::findPtrInfo(T *ptr){
    typename std::list<PtrDetails<T> >::iterator p;
    // Find ptr in refContainer.
    for (p = refContainer.begin(); p != refContainer.end(); p++)
        if (p->memPtr == ptr)
            return p;
    return p; /// Will return end() if not found.
}
// Clear refContainer when program exits.
template <class T, int size>
void Pointer<T, size>::shutdown(){
    if (refContainerSize() == 0)
        return; // list is empty
    typename std::list<PtrDetails<T> >::iterator p;
    for (p = refContainer.begin(); p != refContainer.end(); p++)
    {
        // Set all reference counts to zero
        p->refcount = 0;
    }
    collect();
}