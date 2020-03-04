#include <iostream>
#include <future>
#include <thread>
#include <functional>
#include "multiThreadTaskQueue.h"

void hello(int i,int j){
    std::cout<<" void(int i,int j):"<<i<<" "<<j<<std::endl;
    return;
}
//auto 返回值 test
auto testAuto(int i) -> decltype(i){
    return i;
}

int testPool(int i){
    std::cout<<"pool : "<<i<<std::endl;
    return i;
}

int main() {
    zy::multiThreadTaskQueue excuter(10);
    for(int i = 0;i<10000;i++){
        excuter.commit(testPool, i);
    }
    std::this_thread::sleep_for(std::chrono::seconds(5));
    return 0;
}