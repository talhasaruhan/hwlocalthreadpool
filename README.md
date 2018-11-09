# Hardware Local Thread Pool

## IMPORTANT NOTE: 

This repo contains the very early version of the project, and at its state, it leaks memory and has a rather uncomfortable API. Since the project is under fast paced development, currently I will be only updating the version of this pool used in my [Matrix Multiplication](https://github.com/talhasaruhan/cpp-matmul) project repository. For now, please refer to MatrixMult/ThreadPool.h in that repo.

**Thread pool that respects cache locality on HyperThreaded CPUs (WIN32 API dependent)**

* Each job is described as an array of N functions. (ideal N=2 for HT)  
* For each job, N threads are created and assigned respective functions.  
* For a given job, all threads are guaranteed to be on the same physical core.  
* No two threads from different jobs are allowed on the same physical core.  

## Why?  
When doing multithreading on cache sensitive tasks, we want to keep threads that operate on same or contiguous memory region on the same physical core.  

### Matrix multiplication example,   
Assume that we have a HyperThreading system with K physical and 2K logical processors. Each task computes a blocks on the A*B matrix, so memory accesses are exclusive.
If we instantiate 2K threads and leave it to the OS to handle them, in the best case scenerio, each hardware core will have half of its cache available for a block. So, we'll need to access IO more often, possibly freezing the pipeline.  

So, instead, we split these blocks into two and have two threads on the same core work on them. Memory fetches for one thread will directly benefit the other thread as well. Very much contrary to the previous loosely threaded scenerio.  

Note that we don't account for context switches etc. Also, there may be more complex CPU trickery I don't account for. 
If you think I missed or am wrong about something, feel free to contact me. 
But at the end of the day, I've empirically shown that this works better in my matrix multiplication example.

## Examples

### Example for QueryHWCores
```
#include <Windows.h>

/************/

/* create placeholder mask */
ULONG_PTR mask;
/* id of the core we want to query */
const unsigned CPU_ID = 0;

int maskQueryRet = QueryHWCores::GetProcessorMask(CPU_ID, mask);

if (maskQueryRet) {
    std::cerr << maskQueryRet;
    return;
}

/* make sure this thread is run on CPU #CPU_ID
SetThreadAffinityMask(GetCurrentThread(), mask);

```

### Example for HWLocalThreadPool

```
/* HWLocalThreadPool<NumPhysicalCores, NumThredsPerCore, Debug>                   
Set NumPhysicalCores = -1 to use all physical cores on the current system 
Note that these are queried on the run-time. */

HWLocalThreadPool<2, 2, 0> pool;

/* make sure the arrray doesn't go out of scope during the pools lifetime */
function<void()> arr1[3] = {
    HWLocalThreadPool<>::WrapFunc(foo, 4),
    HWLocalThreadPool<>::WrapFunc(foo, 1)
};
function<void()> arr2[3] = {
    HWLocalThreadPool<>::WrapFunc(foo, 2),
    HWLocalThreadPool<>::WrapFunc(foo, 1)
};
function<void()> arr3[3] = {
    HWLocalThreadPool<>::WrapFunc(foo, 3),
    HWLocalThreadPool<>::WrapFunc(foo, 1)
};

/* submit jobs */
pool.Add(arr1);
pool.Add(arr2);
pool.Add(arr3);

/* when in a local scope, create arrays on the heap */
{
  function<void()>* arr4 = new std::function<void()>[4];
  arr4[0] = std::move(HWLocalThreadPool<>::WrapFunc(foo, 3));
  arr4[1] = std::move(HWLocalThreadPool<>::WrapFunc(foo, 1));
  pool.Add(arr4);
}

/* close the pool! make sure you do this. */
pool.Close();
```
