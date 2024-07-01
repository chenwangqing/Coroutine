/**
 * @file     Universal.h
 * @brief    通用接口
 * @author   CXS (chenxiangshu@outlook.com)
 * @version  1.0
 * @date     2024-07-01
 *
 * @copyright Copyright (c) 2024  Four-Faith
 *
 * @par 修改日志:
 * <table>
 * <tr><th>日期       <th>版本    <th>作者    <th>说明
 * <tr><td>2024-07-01 <td>1.0     <td>CXS     <td>创建
 * </table>
 */
#if !defined(__UNIVERSAL_H__)
#define __UNIVERSAL_H__
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// 虚拟文件通用接口
/// </summary>
typedef struct
{
    // FUNC：GetMillisecond
    // NOTE：获取毫秒值
    uint64_t (*GetMillisecond)(void);

    // FUNC：Malloc
    // NOTE：内存分配
    void *(*Malloc)(size_t size, const char *file, int line);

    // FUNC：Free
    // NOTE：内存释放
    void (*Free)(void *obj, const char *file, int line);

    // FUNC：DelayMs
    // NOTE：延时毫秒
    void (*DelayMs)(uint32_t time);

    // FUNC：CreateLock
    // NOTE：创建锁 (不会重入)
    // RETV：锁对象
    void *(*CreateLock)(const char *name);

    // FUNC：DeleteLock
    // PARS：obj 锁对象
    // NOTE：删除锁
    void (*DeleteLock)(void *obj);

    // FUNC：Lock
    // PARS：锁对象
    // NOTE：加锁
    bool (*Lock)(void *obj, uint32_t timeout);

    // FUNC：UnLock
    // PARS：锁对象
    // NOTE：解锁
    void (*UnLock)(void *obj);

    // FUNC：CreateSemaphore
    // PARS：num 初始值
    // NOTE：创建信号量
    void *(*CreateSemaphore)(const char *name, int num);

    // FUNC：DeleteSemaphore
    // NOTE：删除信号量
    void (*DeleteSemaphore)(void *sem);

    // FUNC：Wait
    // PARS：num 信号数量
    // PARS：time 等待时间 ms 0：不等待
    // NOTE：等待信号量
    bool (*Wait)(void *sem, int num, uint32_t time);

    // FUNC：Give
    // PARS：num 信号数量
    // NOTE：给信号量
    void (*Give)(void *sem, int num);

    // FUNC：RunTask
    // NOTE：运行任务
    // PARS：fun 任务函数
    // PARS：pars 参数
    // PARS: stacks 堆栈大小 0：默认大小
    // PARS: name 任务名称
    // RETV：任务ID  0: 表示失败
    size_t (*RunTask)(void (*fun)(void *pars), void *pars, uint32_t stacks, const char *name);

    // FUNC：GetTaskId
    // NOTE：获取当前任务ID / 线程id
    // RETV：任务ID
    size_t (*GetTaskId)(void);

    // FUNC：Async
    // NOTE：异步调用
    // PARS：fun 任务函数 返回结果指针
    // PARS：pars 参数
    // PARS: stacks 堆栈大小 0：默认大小
    // RETV：异步对象
    void *(*Async)(void *(*fun)(void *pars), void *pars, uint32_t stacks);

    // FUNC：AsyncWait
    // NOTE：等待异步调用结果
    // PARS：async 异步对象
    // PARS：timeout 超时时间 ms 0：不超时
    // RETV：true：成功 false：超时
    bool (*AsyncWait)(void *async, uint32_t timeout);

    // FUNC：AsyncGetResult
    // NOTE：获取异步调用结果 (并删除异步对象)
    // PARS：async 异步对象
    // RETV：结果
    void *(*AsyncGetResultAndDelete)(void *async);

    // FUNC：FeedDog
    // NOTE：喂狗
    // PARS：timeout 超时时间 ms 0：禁用看门狗
    // NOTE：用于定时喂狗，防止死锁
    void (*FeedDog)(uint32_t timeout);
} Universal;
#ifdef __cplusplus
}
#endif
#endif   // __UNIVERSAL_H__
