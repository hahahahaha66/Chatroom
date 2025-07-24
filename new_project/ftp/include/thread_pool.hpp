//thread_pool.hpp

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <functional>
#include <future>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

const int MAX_NUM = 12;

class thread_pool {
public:
    thread_pool();
    
    ~thread_pool();

    template<typename T,typename Func,typename... Args>
    auto push_task(Func T::*func, T* obj, Args&&... args) 
    -> std::future<typename std::invoke_result<Func T::*, T&, Args...>::type> {
        using ReturnType = typename std::invoke_result<Func T::*, T&, Args...>::type;

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(func, obj, std::forward<Args>(args)...)
        );
        std::future<ReturnType> result = task->get_future();
        {
            std::unique_lock<std::mutex> lock(mutex);
            workqueue.push([task]() { (*task)(); });
        }
        cond_work.notify_one();
        return result;
    }

private:
    void work();

    std::queue<std::function<void()>> workqueue;
    bool stop;
    std::vector<std::thread> threads;
    std::mutex mutex;
    std::condition_variable cond_work;
};

#endif // THREAD_POOL_H
