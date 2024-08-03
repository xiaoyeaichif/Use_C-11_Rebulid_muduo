#pragma once

#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread
{
    // __thread 是一个编译器扩展，
    // 表示 t_cachedTid 是线程局部存储变量，即每个线程都有自己的独立副本。
    extern __thread int t_cachedTid;

    void cacheTid();  // 缓存当前线程id

    inline int tid()
    {
        if (__builtin_expect(t_cachedTid == 0, 0))
        {
            cacheTid(); // 如果 t_cachedTid 为0，调用 cacheTid() 缓存当前线程ID。
        }
        return t_cachedTid;
    }
}