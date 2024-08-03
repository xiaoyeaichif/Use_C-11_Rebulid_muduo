#include "Buffer.h"

#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>

/**
 * 从fd上读取数据  Poller工作在LT模式
 * Buffer缓冲区是有大小的！ 但是从fd上读数据的时候，却不知道tcp数据最终的大小
 */ 
ssize_t Buffer::readFd(int fd, int* saveErrno)
{
    // 引入这个结构，确保不会丢失数据
    char extrabuf[65536] = {0}; // 栈上的内存空间  64K, 出作用域会自动消亡
    
    // struct iovec {
    //     void  *iov_base;    /* Starting address */
    //     size_t iov_len;     /* Number of bytes to transfer */
    // }
    
    struct iovec vec[2];
    
    const size_t writable = writeableBytes(); // 这是Buffer底层缓冲区剩余的可写空间大小
    
    // 指向Buffer中可写入的位置,注意我们用的都是下标来实现的
    vec[0].iov_base = begin() + writerIndex_;
    // 设置可以可以读取的长度为 writable
    vec[0].iov_len = writable;
    
    // 表示Buffer中已经写满，没有位置给你写, 所以其他数据都存在extrabuf中
    vec[1].iov_base = extrabuf;
    // 获取extrabuf的长度，也就是64KB
    vec[1].iov_len = sizeof extrabuf;
    
    /*
        1:如果writable确实小于sizeof extrabuf，这意味着Buffer的剩余空间不足以
        接收所有可能从文件描述符读取的数据。在这种情况下，readv应该使用两个iovec
        结构：第一个是Buffer的剩余部分，第二个是extrabuf。因此，iovcnt被设置为2。

        2:如果writable不小于sizeof extrabuf，这意味着Buffer有足够的空间来接收所
        有可能读取的数据，无需使用extrabuf。在这种情况下，readv只需要使用一个iovec
        结构，即Buffer本身。因此，iovcnt被设置为1
    */


    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;

    // 调用readv函数从fd读取数据到vec指定的多个缓冲区中
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0)
    {
        *saveErrno = errno;
    }
    else if (n <= writable) // Buffer的可写缓冲区已经够存储读出来的数据了
    {
        writerIndex_ += n;
    }
    else // extrabuf里面也写入了数据 
    {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);  // writerIndex_开始写 n - writable大小的数据
    }

    return n;
}

// 将缓冲区的数据写进套接字中
ssize_t Buffer::writeFd(int fd, int* saveErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n < 0)
    {
        *saveErrno = errno;
    }
    return n;
}