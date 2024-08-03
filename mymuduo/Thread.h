#pragma once

#include "noncopyable.h"

#include <functional>
#include <thread>
#include <memory>
#include <unistd.h>
#include <string>
#include <atomic>

class Thread : noncopyable
{
public:
    using ThreadFunc = std::function<void()>; // 生成一个函数对象，返回值为void,参数为空
    // 构造函数，不允许隐式转换，初始化线程时，也初始化线程ID
    explicit Thread(ThreadFunc, const std::string &name = std::string());
    ~Thread();

    void start(); // 开启线程
    void join(); // 等待线程执行完毕

    bool started() const { return started_; } // 检查线程是否已经启动
    pid_t tid() const { return tid_; } // 得到当前线程的ID
    const std::string& name() const { return name_; } // 设置当前线程名字

    static int numCreated() { return numCreated_; } // 获取线程的创建的数量
private:
    void setDefaultName(); // 设置默认名称

    bool started_;
    bool joined_;
    std::shared_ptr<std::thread> thread_;  //便于我们自己掌控线程启动的时机
    pid_t tid_;
    ThreadFunc func_; // 存储线程函数
    std::string name_;
    static std::atomic_int numCreated_; // 用来记录线程产生的个数
};