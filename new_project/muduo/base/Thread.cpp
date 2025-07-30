#include "Thread.h"
#include "CurrentThread.h"

#include <cstdio>
#include <memory>
#include <semaphore.h>

std::atomic_int32_t Thread::numCreated_(0);

Thread::Thread(ThreadFunc func, const std::string &name) :
    started_(false),
    joined_(false),
    tid_(0),
    func_(std::move(func)),
    name_(name)
{
    setDefaultName();
}

Thread::~Thread() 
{   
    //如果线程启动并且未等待，则将线程分离，等待系统自动回收
    if(started_ && !joined_)
    {
        thread_->detach();
    }
}

void Thread::start()
{
    started_ = true;
    //使用信号量来通知和唤醒
    sem_t sem;
    sem_init(&sem, false, 0);
    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){
        tid_ = CurrentThread::tid();    //子线程的线程id

        sem_post(&sem);     //唤醒主线程，表示tid_t准备好了(也就是子线程)

        func_();    //执行线程任务
    }));
    sem_wait(&sem);  //若子线程未准备好，主线程会一直阻塞在这里
}

void Thread::join()
{
    joined_ = true;
    thread_->join();
}

void Thread::setDefaultName()
{
    int num = ++numCreated_;
    if (name_.empty())
    {
        char buf[32] = {0};
        snprintf(buf, sizeof(buf), "Thread%d", num);
        name_ = buf;
    }
}