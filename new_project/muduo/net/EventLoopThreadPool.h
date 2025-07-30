#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOOL_H

#include <cstddef>
#include <string>
#include <vector>
#include <memory>
#include <functional>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool
{
public:
    //创建EventLoop时的初始化回调
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg);
    ~EventLoopThreadPool();

    //设置线程池线程数量
    void setThreadNum(int numThreads) { numThreads_ = numThreads; }

    //启动线程池
    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    EventLoop* getNextLoop();  //获取一个工作的EventLoop

    std::vector<EventLoop*> getAllLoops();  //获取所有的EventLoop

    bool started() const { return started_; }  //判断是否启动线程池

    const std::string name() const { return name_; }  //返回线程池名字

private:
    EventLoop* baseLoop_;  //主线程的EventLoop，管理多个EventLoopThread
    std::string name_;  //线程池名字
    bool started_;  //是否启动线程池
    int numThreads_;  //线程池中的数量，一个线程一个EventLoop
    size_t next_;  //当前轮询的线程下标
    std::vector<std::unique_ptr<EventLoopThread>> threads_;  //保存所有的EventLoopThread
    std::vector<EventLoop*> loops_;  //保存所有的EventLoop
};

#endif