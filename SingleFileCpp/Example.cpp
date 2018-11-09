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

    /* HWLocalThreadPool<NumPhysicalCores, NumThredsPerCore>                   
    Set NumPhysicalCores = -1 to use all physical cores on the current system 
    Note that these are queried on the run-time. */

    HWLocalThreadPool<1, 1> pool;

    /* submit jobs */
    pool.Add({
        HWLocalThreadPool<>::WrapFunc(foo, 2),
        HWLocalThreadPool<>::WrapFunc(foo, 3)
        });
    //pool.Add({
    //    HWLocalThreadPool<>::WrapFunc(foo, 4),
    //    HWLocalThreadPool<>::WrapFunc(foo, 1)
    //    });
    //pool.Add({
    //    HWLocalThreadPool<>::WrapFunc(foo, 1),
    //    HWLocalThreadPool<>::WrapFunc(foo, 5)
    //    });


    pool.Close();
}