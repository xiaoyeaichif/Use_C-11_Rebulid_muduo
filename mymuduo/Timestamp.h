#pragma once

#include <iostream>
#include <string>

// 时间类
class Timestamp
{
public:
    Timestamp();
    // 关键字 explicit 防止该构造函数进行隐式类型转换。
    explicit Timestamp(int64_t microSecondsSinceEpoch);
    /*
    *   静态成员函数，用于获取当前时间的时间戳。
    *   返回一个 Timestamp 对象，表示当前时刻。
    */ 
    static Timestamp now();
    // 将长整型的代码改成字符串
    std::string toString() const;
private:
    int64_t microSecondsSinceEpoch_;
};