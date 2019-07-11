# poolAllocator
pool allocator optimized for speed and optimized for size

IDE:
Eclipse IDE for C/C++ Developers
Version: Mars.2 Release (4.5.2)
Build id: 20160218-0600

TREE
```C++
----poolAllocator
    |----armPA
        |----.settings
        |----inc
	    |----armPA.ipp
        |----src
	    |----main.cpp
        |----test
	    |----example.h
	    |----unitTest.cpp
    |----README.md
```

GCC C++ Compiler & MinGW C++ Linker: 
```C++
g++ -I../inc -I../test -O0 -g3 -Wall -c -fmessage-length=0 -std=gnu++11 -o "src\\main.o" "..\\src\\main.cpp" 
```

INSTRUCTIONS:
------------------------------------------------------------------------------------------------------------------------------------
More unit tests can be added to main.cpp. Enter data types as template to run the test.
Example: To run test for data type : someDataType_t, execute
```C++
UNITTEST unitTest;
unitTest.run<int, float, someDataType_t>();  
```
OR
```C++
UNITTEST unitTest;
unitTest.run<someDataType_t>();
```

------------------------------------------------------------------------------------------------------------------------------------
inc/armPA.ipp contains the pool allocator code.
2 implementations are done.

1. OPTIMIZED_FOR_SIZE: run on memory constrained application. This is has a better interface according to me.  
   (To use this implementation, uncomment #define OPTIMIZED_FOR_SIZE in this file)
   WARNING: Slow execution

    Usage: (allocating int data type)
```C++
      PoolAlloc poolAl;
      poolAl.CreatePool<int>();                       ...     (1)
      int* ptr = poolAl.Allocate<int>();              ...     (2)
      poolAl.Deallocate(ptr);                         ...     (3)
      poolAl.DeletePool<int>();                       ...     (4)
```
2. OPTIMIZED_FOR_SPEED: This is the default implementation and is optimized for speed. 
    (To use this implementation, comment #define OPTIMIZED_FOR_SIZE in this file)

    Usage: (allocating int data type)
```C++
      PoolAlloc poolAl;
      const void* int_h = poolAl.CreatePool<int>();   ...     (5)
      int* ptr = poolAl.Allocate<int>(&int_h);        ...     (6)
      poolAl.Deallocate(&int_h, ptr);                 ...     (7)
      poolAl.DeletePool<int>(&int_h);                 ...     (8)
```

     (1) & (5)     Creating a pool allocator
     (2) & (6)     Allocating in an existing pool
     (3) & (7)     Deallocating from an existing pool
     (4) & (8)     Deleting an existing pool
Size of the pool can be edited in armPA.ipp:

```C++
const int g_MaxNumberOfObjectsInPool = 1000;
```

------------------------------------------------------------------------------------------------------------------------------------
unitTest.cpp contains tests that can be modified. Existing tests can be modified by commenting the preprocessor directives
```C++
//  TESTS TO RUN FOR EACH DATA TYPE
#define CREATING_AND_DELETING_A_POOL_EX_TIME
#define ALLOCATING_POOLS_EX_TIME
#define ALLOCATING_POOLS_PTR_CHECK
#define ALLOCATING_AND_DEALLOCATING_POOLS_EX_TIME
//  FOR COMPARISON
#define MALLOC_EX_TIME
#define MALLOC_AND_FREE
```

Test cycles can be modified within each test fixtures
```C++
unsigned int testCycles;
```

------------------------------------------------------------------------------------------------------------------------------------
example.h contains the provided data types. More data types can be added here and tested from main.cpp
