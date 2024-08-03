#pragma once

#include "noncopyable.h"
#include "Timestamp.h"

#include <functional>
#include <memory>

class EventLoop;

/**
 * 理清楚  EventLoop、Channel、Poller之间的关系   《= Reactor模型上对应 Demultiplex
 * Channel 理解为通道，封装了sockfd和其感兴趣的event，如EPOLLIN、EPOLLOUT事件
 * 还绑定了poller返回的具体事件
 */ 
class Channel : noncopyable
{
public:
    //  事件的回调
    using EventCallback = std::function<void()>;
    //  只读事件的回调
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop *loop, int fd);
    ~Channel();

    // fd得到poller通知以后，处理事件的
    void handleEvent(Timestamp receiveTime);  

    // 设置回调函数对象
    // 设置读回调 cb是临时对象，直接转移的话提高了系统的性能，少了赋值构造和析构的过程
    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
    //设置写回调
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    // 设置关闭回调
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    // 设置错误回调
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    // 防止当channel被手动remove掉，channel还在执行回调操作
    void tie(const std::shared_ptr<void>&);

    int fd() const { return fd_; } // 返回文件描述符
    int events() const { return events_; }
    int set_revents(int revt) { revents_ = revt; return revents_;}

    // 设置fd相应的事件状态
    //  能读
    void enableReading() { events_ |= kReadEvent; update(); }
    //  不能读
    void disableReading() { events_ &= ~kReadEvent; update(); }
    //  能写
    void enableWriting() { events_ |= kWriteEvent; update(); }
    //  不能写
    void disableWriting() { events_ &= ~kWriteEvent; update(); }
    //  所有操作都结束
    void disableAll() { events_ = kNoneEvent; update(); }

    // 返回fd当前的事件状态
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }

    // one loop per thread
    EventLoop* ownerLoop() { return loop_; }  // 一个线程负责一种功能
    void remove();
private:

    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent;  //都不感兴趣
    static const int kReadEvent;  //对读事件感兴趣
    static const int kWriteEvent; //对写事件感兴趣

    EventLoop *loop_; // 事件循环
    const int fd_;    // fd, Poller监听的对象
    int events_; // 注册fd感兴趣的事件
    int revents_; // poller返回的具体发生的事件
    int index_;

    std::weak_ptr<void> tie_; // 防止手动
    bool tied_;

    // 因为channel通道里面能够获知fd最终发生的具体的事件revents，
    // 所以它负责调用具体事件的回调操作
    ReadEventCallback readCallback_;
    EventCallback writeCallback_; //写回调
    EventCallback closeCallback_; //关闭回调
    EventCallback errorCallback_; //错误回调
};

