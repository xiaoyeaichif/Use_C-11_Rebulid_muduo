#include "Thread.h"
#include "CurrentThread.h"

#include <semaphore.h>

std::atomic_int Thread::numCreated_(0); // 静态成员变量类外初始化

Thread::Thread(ThreadFunc func, const std::string &name)
    : started_(false) // 标志线程是否在运行
    , joined_(false)
    , tid_(0)
    , func_(std::move(func)) // 提高效率，底层的func直接把资源转给func_,而不是通过副本传递
    , name_(name)
{
    setDefaultName();
}

Thread::~Thread()
{
    if (started_ && !joined_) // 如果线程已经启动并且还没有结束，分离线程，防止死锁的发生
    {
        thread_->detach(); // thread类提供的设置分离线程的方法
    }
}

void Thread::start()  // 一个Thread对象，记录的就是一个新线程的详细信息
{
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0); // 初始化线程的信号量

    // 开启线程
    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){
        // 获取线程的tid值
        tid_ = CurrentThread::tid();
        sem_post(&sem);
        // 开启一个新线程，专门执行该线程函数
        func_(); 
    }));

    // 这里必须等待获取上面新创建的线程的tid值
    sem_wait(&sem);
}

void Thread::join()
{
    joined_ = true;
    thread_->join(); // 等待线程结束
}

void Thread::setDefaultName()
{
    int num = ++numCreated_;
    if (name_.empty()) // 如果线程都还没有名字，给线程一个名字，线程名字就是线程的ID
    {
        char buf[32] = {0};
        snprintf(buf, sizeof buf, "Thread%d", num); 
        name_ = buf;
    }
}