//
// Created by mars on 2020/3/4.
//

#ifndef _MULTITHREADTASKQUEUE_MULTITHREADTASKQUEUE_H
#define _MULTITHREADTASKQUEUE_MULTITHREADTASKQUEUE_H

#include <queue>
#include <functional>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <future>
#include <iostream>
#include <unordered_map>

namespace zy{
    class multiThreadTaskQueue {
    private:
        std::queue<std::function<void()>> taskQueue;
        std::condition_variable hasTaskCV;
        std::atomic_bool isRunning ;
        std::vector<std::thread> threadPool;
        std::mutex taskMutex;
        std::unordered_map<std::thread::id,int> totalMap;
    public:
        multiThreadTaskQueue(int size){
            int currentConcurrency = std::thread::hardware_concurrency();
            int realCreate = 1;
            if(size > currentConcurrency){
                std::cout<<"only "<<currentConcurrency<<" thread are created"<<std::endl;
                realCreate = currentConcurrency;
            }
            isRunning = true;
            for(int i=0;i<realCreate;i++){
                threadPool.emplace_back([this]() {
                    totalMap.emplace(std::this_thread::get_id(),0);
                    while (isRunning) {
                        std::function<void()> task; // 获取一个待执行的 task
                        {
                            std::unique_lock <std::mutex> lock{taskMutex};
                            hasTaskCV.wait(lock, [this] {
                                return !isRunning||!taskQueue.empty();
                            }); // wait 直到有 task
                            if (!isRunning && taskQueue.empty()){
                                return;
                            }//线程结束

                            task = move(taskQueue.front()); // 按先进先出从队列取一个 task
                            taskQueue.pop();
                            totalMap[std::this_thread::get_id()]++;
//                            std::cout<<"thread "<<std::this_thread::get_id()<<" is runing "<<std::endl;
                        }
                        task();//执行任务
                    }
                });
            }
        }
        template <typename FUNC,typename... ARGS>
        auto commit(FUNC && func, ARGS &&...args) ->std::future<decltype(func(args...))>{
            if(isRunning == false){
                throw std::runtime_error("initTask first");
            }
            using taskRetType = decltype(func(args...));
            auto tempTask = std::make_shared<std::packaged_task<taskRetType()>>(
                    std::bind(std::forward<FUNC>(func), std::forward<ARGS>(args)...));
            std::future<taskRetType> taskFuture = tempTask->get_future(); //返回值
            {
                std::lock_guard<std::mutex> lckMutex(taskMutex);
                taskQueue.emplace([tempTask](){
                    (*tempTask)();        //封装成一个void()任务
                });
            }
            hasTaskCV.notify_one();
            return taskFuture;
        }
        ~multiThreadTaskQueue(){
            isRunning = false;
            hasTaskCV.notify_all();
            for(int i =0;i<threadPool.size();i++){
                if(threadPool[i].joinable()){
                    threadPool[i].join();
                }
            }
            int total = 0;
            for(auto iter:totalMap){
                std::cout<<"thread "<<iter.first<<" : "<<iter.second<<std::endl;
                total+= iter.second;
            }
            std::cout<<std::endl;
            std::cout<<"total count : "<<total<<std::endl;
        }
    };

}



#endif //_MULTITHREADTASKQUEUE_MULTITHREADTASKQUEUE_H
