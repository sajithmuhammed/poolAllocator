#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <typeinfo>
#include <chrono>
#include <ctime>
#include <memory.h>
#include "example.h"

//  TESTS TO RUN FOR EACH DATA TYPE
#define CREATING_AND_DELETING_A_POOL_EX_TIME
#define ALLOCATING_POOLS_EX_TIME
#define ALLOCATING_POOLS_PTR_CHECK
#define ALLOCATING_AND_DEALLOCATING_POOLS_EX_TIME
//  FOR COMPARISON
#define MALLOC_EX_TIME
#define MALLOC_AND_FREE

class UNITTEST{
  unsigned int nrOfTestValues;
  PoolAlloc* pAlloc;
  unsigned int cases;
  unsigned int passed;
  unsigned int failed;
public:
  UNITTEST() :
    nrOfTestValues(0),
    pAlloc(new PoolAlloc()),
    cases(0),
    passed(0),
    failed(0)
  {}

  template<typename... T1>
  void run()
  {
      nrOfTestValues = sizeof...(T1);
      std::cout << "[UNIT TEST]\t" << "starting..." << std::endl;
      run<T1..., unsigned int>(nrOfTestValues);
      std::cout << "\n[UNIT TEST]" << "\tPASSED: " << passed << "\tFAILED: " << failed << std::endl << std::endl << std::endl;
      passed = 0;
      failed = 0;
  }

  template<typename T, typename... T1, typename C>
  void run(C count)
  {
      // template recursion
      if(--count > 0)
        run<T1..., C>(count);

      std::cout << "\n\n" << ++cases << ". Test Cases for type ID : " << typeid(T).name() << std::endl;
      unsigned int testCycles;
      bool status;
      T* t, *tPrevious;
      const void* hT, *hTPrevious;

      std::chrono::system_clock::time_point start, end;
      std::chrono::duration<double> elapsed_seconds;
      std::cout << "----------------------------------" << std::endl;


#if defined(CREATING_AND_DELETING_A_POOL_EX_TIME)
      /**************************************************************************/
      std::cout << "[START]\tCREATING AND DELETING A POOL(Execution Time): " << std::endl << std::endl;
      // Tear up
      testCycles = 100000;
      status = true;
      //

      start = std::chrono::system_clock::now();
      for(unsigned int i=0; i<testCycles; i++)
      {
#if defined(OPTIMIZED_FOR_SIZE)
          try
          {
              pAlloc->CreatePool<T>();
              pAlloc->DeletePool<T>();
          }
          catch(...)
          {
              status &= false;
          }
#else
          if((hT=pAlloc->CreatePool<T>()) == nullptr){
            status &= false;
            break;
          }
          else
          {
              try
              {
                  pAlloc->DeletePool<T>(&hT);
              }
              catch(...)
              {
                  status &= false;
              }
          }
#endif
      }
      end = std::chrono::system_clock::now();

      elapsed_seconds = end - start;
      if(status)
      {
          std::cout<< " Execution time : " << (elapsed_seconds).count() << std::endl;
      }
      std::cout << " Cycles executed: "<< testCycles << std::endl;
      std::cout << "\n[END]\t" << (status?"PASSED":"FAILED") << std::endl;
      std::cout << "--------------------------" << std::endl;
      if(!status)
        failed++;
      else
        passed++;
      // Tear down
      //
#endif
      /**************************************************************************/
#if defined(ALLOCATING_POOLS_EX_TIME)
      std::cout << "[START]\tALLOCATING POOLS(Execution Time): " << std::endl << std::endl;
      // Tear up
      testCycles = 500000;
      status = true;
#if defined(OPTIMIZED_FOR_SIZE)
      pAlloc->CreatePool<T>();
#else
      hT = pAlloc->CreatePool<T>();
#endif
      //

      start = std::chrono::system_clock::now();
      for(unsigned int i=1; i<testCycles; i++)
      {
#if defined(OPTIMIZED_FOR_SIZE)
          if((t=pAlloc->Allocate<T>()) == nullptr){
#else
          if((t=pAlloc->Allocate<T>(&hT)) == nullptr){
#endif
            status &= false;
            break;
          }
      }
      end = std::chrono::system_clock::now();
      elapsed_seconds = end - start;

      if(status)
      {
          std::cout<< " Execution time : " << (elapsed_seconds).count() << std::endl;
      }
      std::cout << " Cycles executed: "<< testCycles << std::endl;
      std::cout << "\n[END]\t" << (status?"PASSED":"FAILED") << std::endl;
      std::cout << "--------------------------" << std::endl;
      if(!status)
        failed++;
      else
        passed++;
      // Tear down
#if defined(OPTIMIZED_FOR_SIZE)
      pAlloc->DeletePool<T>();
#else
      pAlloc->DeletePool<T>(&hT);
#endif
      //
#endif
      /**************************************************************************/
#if defined(MALLOC_EX_TIME)
      std::cout << "[START]\tMALLOC(Execution Time): " << std::endl << std::endl;
      // Tear up
      testCycles = 500000;
      status = true;
      //

      T* tArr[testCycles];
      start = std::chrono::system_clock::now();
      for(unsigned int i=1; i<testCycles; i++)
      {
          if((tArr[i]=(T*)malloc(sizeof(T))) == nullptr){
            status &= false;
            break;
          }
      }
      end = std::chrono::system_clock::now();
      elapsed_seconds = end - start;

      if(status)
      {
          std::cout<< " Execution time : " << (elapsed_seconds).count() << std::endl;
      }
      std::cout << " Cycles executed: "<< testCycles << std::endl;
      std::cout << "\n[END]\t" << (status?"PASSED":"FAILED") << std::endl;
      std::cout << "--------------------------" << std::endl;
      // Tear down
      for(unsigned int i=1; i<testCycles; i++)
      {
          free(tArr[i]);
      }
      //
#endif
      /**************************************************************************/
#if defined(ALLOCATING_POOLS_PTR_CHECK)
      std::cout << "[START]\tALLOCATING POOLS(Pointer returned): " << std::endl << std::endl;
      // Tear up
      testCycles = g_MaxNumberOfObjectsInPool;
      status = true;
#if defined(OPTIMIZED_FOR_SIZE)
      pAlloc->CreatePool<T>();
#else
      hT = pAlloc->CreatePool<T>();
#endif
      //

      // To check if returns a valid pointers
#if defined(OPTIMIZED_FOR_SIZE)
      tPrevious = pAlloc->Allocate<T>();
      for(unsigned int i=1; i<g_MaxNumberOfObjectsInPool; i++)
      {
        if((t=pAlloc->Allocate<T>()) == nullptr){
          status &= false;
          break;
        }
        if(((char*)t)-((char*)tPrevious) != sizeof(T)+(sizeof(T)==1?1:0))
          status &= false;
        tPrevious = t;
      }
#else
      hTPrevious = hT;
      tPrevious = pAlloc->Allocate<T>(&hT);
      if((unsigned int)tPrevious != (unsigned int)(&((PoolAlloc::dType_t<T>*)hTPrevious)->data))
        status &= false;
      for(unsigned int i=1; i<g_MaxNumberOfObjectsInPool; i++)
      {
        if((t=pAlloc->Allocate<T>(&hT)) == nullptr){
          status &= false;
          break;
        }
        if(((char*)t)-((char*)tPrevious) != sizeof(PoolAlloc::dType_t<T>))
          status &= false;
        tPrevious = t;
      }
#endif

      std::cout << " Cycles executed: "<< testCycles << std::endl;
      std::cout << "\n[END]\t" << (status?"PASSED":"FAILED") << std::endl;
      std::cout << "--------------------------" << std::endl;
      if(!status)
        failed++;
      else
        passed++;
      // Tear down
#if defined(OPTIMIZED_FOR_SIZE)
      pAlloc->DeletePool<T>();
#else
      pAlloc->DeletePool<T>(&hT);
#endif
      //
#endif
      /**************************************************************************/
#if defined(ALLOCATING_AND_DEALLOCATING_POOLS_EX_TIME)
      std::cout << "[START]\tALLOCATING AND DEALLOCATING POOLS(Execution Time): " << std::endl << std::endl;
      // Tear up
      testCycles = 50000000;
      status = true;
#if defined(OPTIMIZED_FOR_SIZE)
      pAlloc->CreatePool<T>();
#else
      hT = pAlloc->CreatePool<T>();
#endif
      //

      start = std::chrono::system_clock::now();
      for(unsigned int i=1; i<testCycles; i++)
      {
#if defined(OPTIMIZED_FOR_SIZE)
          if((t=pAlloc->Allocate<T>()) == nullptr){
            status &= false;
            break;
          }
          pAlloc->Deallocate(t);
#else
          if((t=pAlloc->Allocate<T>(&hT)) == nullptr){
            status &= false;
            break;
          }
          pAlloc->Deallocate(&hT, t);
#endif
      }
      end = std::chrono::system_clock::now();
      elapsed_seconds = end - start;

      if(status)
      {
          std::cout<< " Execution time : " << (elapsed_seconds).count() << std::endl;
      }
      std::cout << " Cycles executed: "<< testCycles << std::endl;
      std::cout << "\n[END]\t" << (status?"PASSED":"FAILED") << std::endl;
      std::cout << "--------------------------" << std::endl;
      if(!status)
        failed++;
      else
        passed++;
      // Tear down
#if defined(OPTIMIZED_FOR_SIZE)
      pAlloc->DeletePool<T>();
#else
      pAlloc->DeletePool<T>(&hT);
#endif
      //
#endif
      /**************************************************************************/
#if defined(MALLOC_AND_FREE)
      std::cout << "[START]\tMALLOC AND FREE(Execution Time): " << std::endl << std::endl;
      // Tear up
      testCycles = 50000000;
      status = true;
      //

      start = std::chrono::system_clock::now();
      for(unsigned int i=1; i<testCycles; i++)
      {
          if((t=(T*)malloc(sizeof(T))) == nullptr){
            status &= false;
            break;
          }
          free(t);
      }
      end = std::chrono::system_clock::now();
      elapsed_seconds = end - start;

      if(status)
      {
          std::cout<< " Execution time : " << (elapsed_seconds).count() << std::endl;
      }
      std::cout << " Cycles executed: "<< testCycles << std::endl;
      std::cout << "\n[END]\t" << (status?"PASSED":"FAILED") << std::endl;
      std::cout << "--------------------------" << std::endl;
      // Tear down
      //
#endif
      /**************************************************************************/
  }
};
