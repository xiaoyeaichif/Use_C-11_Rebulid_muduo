#pragma once

#include "noncopyable.h"

class InetAddress; // 前置声明

// 封装socket fd
class Socket : noncopyable
{
public:
    explicit Socket(int sockfd)
        : sockfd_(sockfd)
    {}

    ~Socket();
    /*
        服务器的流程1：创建socket---->2：绑定IP+端口
        ---->3：监听端口是否有连接---->4：接收端口连接

    */
    int fd() const { return sockfd_; } // 只读
    void bindAddress(const InetAddress &localaddr); // 绑定IP+PORT
    void listen(); // 监听端口是否有连接
    int accept(InetAddress *peeraddr); // 完成连接

    void shutdownWrite();

    // 设置 TCP_NODELAY 选项（关闭 Nagle 算法）
    void setTcpNoDelay(bool on);

    // 设置 SO_REUSEADDR 选项
    void setReuseAddr(bool on);

    // 设置 SO_REUSEPORT 选项
    void setReusePort(bool on);

    // 设置 SO_KEEPALIVE 选项
    void setKeepAlive(bool on);
private:
    const int sockfd_;
};