#include <iostream>
#include <string.h>
#include <thread>
#include <chrono>
#include <random>
#include <sstream>
#include <Windows.h>
#include "HWLocalThreadPool.h"

using namespace std;

void foo(int i) {
    std::this_thread::sleep_for(std::chrono::seconds(i));

    HANDLE curHandle = GetCurrentThread();
    ULONG_PTR oldAffinityMask = SetThreadAffinityMask(curHandle , 3);
    SetThreadAffinityMask(curHandle, oldAffinityMask);

    std::stringstream line;
    line << "slept " << i << " seconds " << QueryHWCores::BitmaskToStr(oldAffinityMask) << " " << curHandle << "\n";
    std::cout << line.str();
}

int main() {

    /* HWLocalThreadPool<NumPhysicalCores, NumThredsPerCore, Debug>                   
    Set NumPhysicalCores = -1 to use all physical cores on the current system 
    Note that these are queried on the run-time. */

    HWLocalThreadPool<2, 2, 0> pool;
    
    /* make sure the arrray doesn't go ot of scope */
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
    function<void()>* arr4 = new std::function<void()>[4];
    arr4[0] = std::move(HWLocalThreadPool<>::WrapFunc(foo, 3));
    arr4[1] = std::move(HWLocalThreadPool<>::WrapFunc(foo, 1));
    pool.Add(arr4);

    /* close the pool! make sure you do this. */
    pool.Close();
}