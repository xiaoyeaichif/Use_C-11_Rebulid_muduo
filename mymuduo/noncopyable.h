#pragma once

/*
 * noncopyable被继承以后，派生类对象可以正常的构造和析构，但是派生类对象
 * 无法进行拷贝构造和赋值操作
 */ 
class noncopyable
{
public:
    noncopyable(const noncopyable&) = delete; //拷贝构造
    noncopyable& operator=(const noncopyable&) = delete; //赋值构造
protected:
    noncopyable() = default;  //默认构造函数
    ~noncopyable() = default; //默认析构函数
};