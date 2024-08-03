#pragma once

#include <vector>
#include <string>
#include <algorithm>

// 网络库底层的缓冲器类型定义
class Buffer
{
public:
    static const size_t kCheapPrepend = 8; // 初始的下标的位置，这部分大小是预留的
    static const size_t kInitialSize = 1024; // 缓冲区的初始大小，整体需要加8字节

    // 初始的buffer大小就是1032
    // 读写下标都从8字节的位置开始
    // 构造函数直接初始化了
    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize)
        , readerIndex_(kCheapPrepend)
        , writerIndex_(kCheapPrepend)
    {}

    // 可读的数据数据长度
    size_t readableBytes() const 
    {
        return writerIndex_ - readerIndex_;
    }
    // 缓冲区目前还能写的空间大小
    size_t writeableBytes() const
    {
        return buffer_.size() - writerIndex_;
    }
    // 返回当前读下标的位置,这个有可能不是8字节的，需要注意
    // 返回当前缓冲区中可用于前置插入的字节数。
    size_t prependableBytes() const
    {
        return readerIndex_;
    }

    // 返回缓冲区中可读数据的起始地址
    const char* peek() const
    {
        return begin() + readerIndex_;
    }

    // onMessage string <- Buffer
    // 移动读指针，使已读取的 len 字节数据不再被视为可读数据。
    void retrieve(size_t len)
    {
        // 当前只读了一部分数据, 读指针往后移动，另外缓冲区中还有数据的一部分没读完
        if (len < readableBytes())
        {
            readerIndex_ += len; // 应用只读取了刻度缓冲区数据的一部分，就是len，还剩下readerIndex_ += len -> writerIndex_
        }
        // 读取了缓冲区的所有数据
        // 然后进行复位操作
        else   // len == readableBytes()
        {
            retrieveAll();
        }
    }
    // 缓冲区的复位操作
    void retrieveAll()
    {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    // 把onMessage函数上报的Buffer数据，转成string类型的数据返回
    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes()); // 应用可读取数据的长度
    }

    std::string retrieveAsString(size_t len)
    {
        std::string result(peek(), len);
        retrieve(len); // 上面一句把缓冲区中可读的数据，已经读取出来，这里肯定要对缓冲区进行复位操作
        return result;
    }

    // 往缓冲区写数据
    // buffer_.size() - writerIndex_    len
    /* 
        两种情形
        1：要写的数据长度小于缓冲区的长度,直接写入即可
        2：要写的数据长度大于缓冲区长度,需要扩大容量
    */
    void ensureWriteableBytes(size_t len)
    {
        if (writeableBytes() < len)
        {
            makeSpace(len); // 扩容函数
        }
    }

    // 把[data, data+len]内存上的数据，添加到writable缓冲区当中
    void append(const char *data, size_t len)
    {
        ensureWriteableBytes(len);
        std::copy(data, data+len, beginWrite());
        writerIndex_ += len;
    }

    char* beginWrite()
    {
        return begin() + writerIndex_;
    }

    const char* beginWrite() const
    {
        return begin() + writerIndex_;
    }

    // 从fd上读取数据
    ssize_t readFd(int fd, int* saveErrno);
    // 通过fd发送数据
    ssize_t writeFd(int fd, int* saveErrno);
private:
    // 获取首元素的地址,迭代器变成---->指针的写法
    char* begin()
    {
        // 先获取首元素，然后再取首元素的地址
        // it.operator*()
        return &*buffer_.begin();  // vector底层数组首元素的地址，也就是数组的起始地址
    }
    const char* begin() const
    {
        return &*buffer_.begin();
    }
    // 扩容函数的实现
    void makeSpace(size_t len)
    {
        //  可写的缓冲区大小 + 前置可以插入大小
        if (writeableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            buffer_.resize(writerIndex_ + len); // 扩容
        }
        else
        {
            // 这部函数是将已有数据的区域往前移动，后续可用的区域就会变大
            size_t readalbe = readableBytes(); // 获取已经读取区域的大小
            std::copy(begin() + readerIndex_, 
                    begin() + writerIndex_,
                    begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readalbe; // 下标的变化
        }
    }

    std::vector<char> buffer_;
    size_t readerIndex_; // 写下标
    size_t writerIndex_; // 读下标-----> 已经读取的区域就是 writerIndex_ - readerIndex_
};