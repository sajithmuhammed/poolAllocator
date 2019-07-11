/*****************************************************************************
This file contains the pool allocator code.
2 implementations are done.

1. OPTIMIZED_FOR_SIZE: run on memory constrained application. This is has a better interface according to me.  
   (To use this implementation, uncomment #define OPTIMIZED_FOR_SIZE in this file)
   WARNING: Slow execution

    Usage: (allocating int data type)
      PoolAlloc poolAl;
      poolAl.CreatePool<int>();                       ...     (1)
      int* ptr = poolAl.Allocate<int>();              ...     (2)
      poolAl.Deallocate(ptr);                         ...     (3)
      poolAl.DeletePool<int>();                       ...     (4)

2. OPTIMIZED_FOR_SPEED: This is the default implementation and is optimized for speed. 

    Usage: (allocating int data type)
      PoolAlloc poolAl;
      const void* int_h = poolAl.CreatePool<int>();   ...     (5)
      int* ptr = poolAl.Allocate<int>(&int_h);        ...     (6)
      poolAl.Deallocate(&int_h, ptr);                 ...     (7)
      poolAl.DeletePool<int>(&int_h);                 ...     (8)


     (1) & (5)     Creating a pool allocator
     (2) & (6)     Allocating in an existing pool
     (3) & (7)     Deallocating from an existing pool
     (4) & (8)     Deleting an existing pool
*****************************************************************************/

#ifndef ARMPA_CLASS_H
#define ARMPA_CLASS_H

#include <vector>
#include <map>
#include <stdio.h>
#include <iostream>
#include <typeinfo>
#include <memory.h>
#include <cstdint>
#include "armPA.ipp"

//#define OPTIMIZED_FOR_SIZE

#ifndef OPTIMIZED_FOR_SIZE
#define OPTIMIZED_FOR_SPEED
#endif

const int g_MaxNumberOfObjectsInPool = 1000;

#if defined(OPTIMIZED_FOR_SIZE)
class PoolAlloc{
public:

    template<typename T>
    void CreatePool(void)
    {
        int count = 0;
        for( auto i : addressMap)
        {
            if(i.first == sizeof(T))
                break;
            count++;
        }
        if(count == addressMap.size())
        {
            std::size_t size = g_MaxNumberOfObjectsInPool*sizeof(T);
            if(sizeof(T) == 1)
                size<<=1;
            auto addr = malloc(size);
            if(addr != NULL)
            {
                InitPool((T*)addr);
                auto p = std::pair<void*, void*>(addr, addr);
                addressMap.insert(std::pair<std::size_t, std::pair<void*, void*>>(sizeof(T), p));
            }
            else
            {
                std::cout<<"Error : " << __PRETTY_FUNCTION__ << std::endl);
            }
        }
        else
          std::cout<< "Pool exists" << std::endl;
    }

    template<typename T>
    T* Allocate()
    {
        T* addr = nullptr;
        try
        {
            int increment = 1;
            if(sizeof(T)==1)
              increment = 2;
            addr = (T*)(addressMap.at(sizeof(T))).second;
            addressMap.at(sizeof(T)).second = ((T*)addressMap.at(sizeof(T)).first) + (*(uint16_t*)addr)*increment;
        }
        catch(...)
        {
            std::cout<< "Error: Pool could not be found for type ID :  ", typeid(T).name() << std::endl;
            return nullptr;
        }
        int i;
        for(i = 0; i!=deallocated.size(); i++)
        {
            if(deallocated.at(i) == addr)
            {
                deallocated.erase(deallocated.begin()+i);
                break;
            }
        }
        return addr;
    }


    template<typename T>
    void Deallocate(T* ptr)
    {
        // when allocating back , remove from deallocated vector
        int i;
        for(i = 0; i!=deallocated.size(); i++)
        {
            if(deallocated.at(i) == ptr)
                break;
        }
        if(i != deallocated.size())
        {
            std::cout<<"Error: Already deallocated" << std::endl;
            return;
        }
        try
        {
            auto p = addressMap.at(sizeof(T));
            *(uint16_t*)ptr = (T*)p.second - (T*)(p.first);
        }
        catch(...)
        {
            std::cout<<("Error: " << __PRETTY_FUNCTION__ " : " << typeid(T).name() << std::endl;
            return;
        }

        deallocated.push_back(ptr);

        addressMap.at(sizeof(T)).second = (T*)ptr;
    }

    template<typename T>
    void DeletePool()
    {
        try
        {
            T* addr = (T*)addressMap.at(sizeof(T)).first;
            free(addr);
            addressMap.erase(sizeof(T));
        }
        catch(...)
        {
            std::cout << "Error: Pool not found for deleting ", typeid(T).name() << std::endl;
        }
    }

private:
    std::map<std::size_t, std::pair<void*, void*>> addressMap;
    std::vector<void*> deallocated;

    template<typename T>
    void InitPool(T* addr)
    {
        int index = 1, increment = 1;
        if(sizeof(T) == 1)
            increment = 2;
        for(T* i = addr; i < (addr+g_MaxNumberOfObjectsInPool*increment); i+=increment, index++)
        {
            uint16_t* x = (uint16_t*)i;
            *x = index%g_MaxNumberOfObjectsInPool;
        }
    }
};
#elif defined(OPTIMIZED_FOR_SPEED)

class PoolAlloc{
/********************************************************************************
    dType_t is the place holder for each slots within a pool. 
    A pool will contain g_MaxNumberOfObjectsInPool dType_t structs
    ptr       : Stores address of the next available slot for allocating memory
    data      : This address is returned from the pool when allocation is requested
    typeName  : identifies the type of the data requested (typename T)
********************************************************************************/
  template<typename T>
  struct dType_t{
    unsigned int* ptr;
    T data;
    const char* typeName;//TODO: change this to  4 + 4 bytes .. count the number consumed in one & store sizeof(T) in other
  };   
  
/********************************************************************************
    void InitPool(T* addr);
    This private function is called when a pool needs to be created
    It initializes each of the slots (struct dType_t) within the created pool.
    NOTE: The last slot points to the first slot back again. So user might override
    some data and this is dangerous. User has to keep track and request allocation
    to a maximum value of g_MaxNumberOfObjectsInPool.
********************************************************************************/
  template<typename dType_t>
  void InitPool(dType_t* addr)
  {
    dType_t* i = addr;
    for(; i < (addr+g_MaxNumberOfObjectsInPool)-1; i++)
    {
      i->ptr = (unsigned int *)(i+1);
      i->typeName = typeid(i->data).name();
    }
    i->ptr = (unsigned int*)addr;
    i->typeName = typeid(i->data).name();
  }
  
  std::map<std::string, void*>poolAddr;
public:
   PoolAlloc() : poolAddr(){}
    
/********************************************************************************
    template<typename T>
    const void* CreatePool(void);
    This is the interface function to create a pool of a certain data type with 
    a storage capacity of g_MaxNumberOfObjectsInPool elements. 
    poolAddr    : is used to map the data type to the pools start address. 
                  This is needed for deletion operation only.
    
    return      : returns a handle to of the requested data type to be used for 
                  pool operations such as allocation, deallocation and deletion.
                  Returns nullptr if no memory available for pool.
********************************************************************************/
   template<typename T>
   const void* CreatePool(void)
   {
       std::size_t size = g_MaxNumberOfObjectsInPool * (sizeof(struct dType_t<T>));
       struct dType_t<T>* addr = (dType_t<T>*)malloc(size);
       poolAddr.insert(std::pair<std::string, void*>(typeid(T).name(), (void*)addr));

       if(addr != NULL)
       {
         InitPool<dType_t<T>>(addr);
       }
       else
       {
         std::cout << "Error: " << __PRETTY_FUNCTION__ << " - Could not create pool" << std::endl;
       }
       return (void*)addr;
   }
/********************************************************************************
    template<typename T>
    T* Allocate(const void** dType);
    This is the interface function to allocate an element from the pool. It 
    updates the handle to the next available slot in the pool (which was stored 
    in ptr).
    
    argument 0  : Pointer to the handle for the requested data type
    
    return      : it returns a pointer to a requested data type.
                  Returns nullptr if handle mismatches.
********************************************************************************/
   template<typename T>
   T* Allocate(const void** dType)
   {
       auto addr = (dType_t<T>*)(*dType);
       if(typeid(T).name() != addr->typeName){
         std::cout << "Error: Wrong typeid specified during allocation" << std::endl;
         return nullptr;
       }
       if(addr->ptr == (unsigned int)nullptr){
         std::cout << "Error: nullptr" << std::endl;
         return nullptr;
       }
       *dType = addr->ptr;
       return (T*)&(addr->data);
   }

/********************************************************************************
    template<typename T>
    void Deallocate(const void** dType, T* ptr);
    This is the interface function to deallocate an element from the pool. It
    updates the handle's next available location to the deallocated element.
    ptrAddr_h   : gives the handle address that contains the element requested
                  to be deallocated
    
    argument 0  : Pointer to the handle for the requested data type
    
    argument 1  : pointer to the data type that has to be deallocated
********************************************************************************/
   template<typename T>
   void Deallocate(const void** dType, T* ptr)
   {
       auto addr = (dType_t<T>*)(*dType);
       if(typeid(T).name() != addr->typeName){
         std::cout << "Error: Wrong typeid specified during deallocation" << std::endl;
         return;
       }
       auto ptrAddr_h = (unsigned int)(((unsigned int*)ptr)-1);
       if(ptrAddr_h == (unsigned int)addr)
       {
         std::cout << "\n\tError: Already deallocated\n" << std::endl;
         return;
       }
       else
         *dType = ((unsigned int*)ptr)-1;
       
       (*((dType_t<T>**)dType))->ptr = (unsigned int*)addr;
   }

/********************************************************************************
    template<typename T>
    void DeletePool(const void** dType);
    This is the interface function to delete a pool. Address of the pool is 
    searched in the ordered map and freed. It is erased from the map so any 
    attempt to delete again should fail. Also the handle is updated to nullptr,
    so that the pool cannot be used again.
    
    argument 0  : Pointer to the handle for the requested data type
********************************************************************************/
   template<typename T>
   void DeletePool(const void** dType)
   {
     try
     {
         auto typeName = typeid(T).name();
         free(poolAddr.at(typeName));
         poolAddr.erase(typeName);
         *dType = nullptr;
     }
     catch(...)
     {
         std::cout << "Error: No pool located to delete" << std::endl;
     }
   }
};
#endif

#endif // ARMPA_CLASS_H
