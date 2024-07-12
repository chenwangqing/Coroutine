/**
 * @file     Coroutine.h
 * @brief    通用协程
 * @author   CXS (chenxiangshu@outlook.com)
 * @version  1.20
 * @date     2024-07-10
 *
 * @copyright Copyright (c) 2024  chenxiangshu@outlook.com
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * @par 修改日志:
 * <table>
 * <tr><th>日期       <th>版本    <th>作者    <th>说明
 * <tr><td>2022-08-15 <td>1.0     <td>CXS    <td>创建
 * <tr><td>2022-08-19 <td>1.1     <td>CXS    <td>空闲事件添加休眠时间，用于低功耗
 * <tr><td>2022-08-19 <td>1.2     <td>CXS    <td>VC 编译器 longjmp 会析构C++对象，使用需要自己实现 setjmp / longjmp
 * <tr><td>2022-08-20 <td>1.3     <td>CXS    <td>_Task检查栈是否被迁移
 * <tr><td>2022-08-21 <td>1.4     <td>CXS    <td>优化栈结尾获取方式
 * <tr><td>2022-08-23 <td>1.5     <td>CXS    <td>优化rubbishRecycling减少时间值获取
 * <tr><td>2022-08-23 <td>1.6     <td>CXS    <td>优化_Yield减少上下文切换错误
 * <tr><td>2022-09-09 <td>1.7     <td>CXS    <td>修正DeleteSem没有释放内存的错误
 * <tr><td>2022-09-09 <td>1.8     <td>CXS    <td>添加设置名称接口SetName
 * <tr><td>2022-09-09 <td>1.9     <td>CXS    <td>优化信息打印
 * <tr><td>2022-09-14 <td>1.10    <td>CXS    <td>添加唤醒事件
 * <tr><td>2022-09-19 <td>1.11    <td>CXS    <td>添加默认事件
 * <tr><td>2024-06-26 <td>1.12    <td>CXS    <td>添加Coroutine_Inter
 * <tr><td>2024-06-27 <td>1.13    <td>CXS    <td>修正一些任务切换的错误；添加邮箱通信
 * <tr><td>2024-06-28 <td>1.14    <td>CXS    <td>取消显示协程控制器，自动根据线程id分配控制器
 * <tr><td>2024-07-01 <td>1.15    <td>CXS    <td>添加看门狗；添加优先级
 * <tr><td>2024-07-03 <td>1.16    <td>CXS    <td>添加独立栈
 * <tr><td>2024-07-04 <td>1.17    <td>CXS    <td>独立栈动态分配线程运行
 * <tr><td>2024-07-05 <td>1.18    <td>CXS    <td>删除共享栈，独立栈切换速度快更加实用;添加红黑树用于休眠列表
 * <tr><td>2024-07-08 <td>1.19    <td>CXS    <td>添加 COROUTINE_ENABLE_XXX 宏控制功能开关，方便功能裁剪；完善邮件通信
 * <tr><td>2024-07-10 <td>1.20    <td>CXS    <td>修正跨线程的协程调度错误；完善错误事件；添加通道
 * </table>
 *
 * @note

Coroutine.SetInter(inter);

sem1    = Coroutine.CreateSemaphore("sem1", 0);
mail1   = Coroutine.CreateMailbox("mail1", 1024);
lock    = Coroutine.CreateMutex("lock");

Coroutine_TaskAttribute atr;
memset(&atr, 0, sizeof(Coroutine_TaskAttribute));
atr.co_idx     = -1;
atr.stack_size = 1024 * 16; // 共享栈会自动分配大小，这里可以设置小一些，
atr.pri        = TASK_PRI_NORMAL;

Coroutine.AddTask(Task1, nullptr, "Task1", &atr);

while (true)
    Coroutine.RunTick();
*/

#ifndef __Coroutine_H__
#define __Coroutine_H__
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "Universal.h"
#include "Define.h"

#ifdef __cplusplus
extern "C" {
#endif

// 任务调度时进行栈检查，会增加调度时间开销但能及时发现栈溢出的错误，适用于开发阶段
#ifndef COROUTINE_CHECK_STACK
#define COROUTINE_CHECK_STACK 0
#endif
// 启用邮箱
#ifndef COROUTINE_ENABLE_MAILBOX
#define COROUTINE_ENABLE_MAILBOX 1
#endif
// 启用信号量
#ifndef COROUTINE_ENABLE_SEMAPHORE
#define COROUTINE_ENABLE_SEMAPHORE 1
#endif
// 启用互斥锁
#ifndef COROUTINE_ENABLE_MUTEX
#define COROUTINE_ENABLE_MUTEX 1
#endif
// 启用异步任务
#ifndef COROUTINE_ENABLE_ASYNC
#define COROUTINE_ENABLE_ASYNC 1
#endif
// 启用看门狗
#ifndef COROUTINE_ENABLE_WATCHDOG
#define COROUTINE_ENABLE_WATCHDOG 1
#endif
// 启用打印信息
#ifndef COROUTINE_ENABLE_PRINT_INFO
#define COROUTINE_ENABLE_PRINT_INFO 1
#endif
// 启用管道
#ifndef COROUTINE_ENABLE_CHANNEL
#define COROUTINE_ENABLE_CHANNEL 1
#endif

// -------------- 独立栈协程 --------------
// 优点：切换速度快
// 缺点：占用内存大，容易造成栈溢出，某个任务都需要分配较大的栈空间

#define COROUTINE_VERSION "1.20"

typedef struct _CO_Thread    *Coroutine_Handle;      // 协程实例
typedef struct _CO_TCB       *Coroutine_TaskId;      // 任务id
typedef struct _CO_Semaphore *Coroutine_Semaphore;   // 信号量
typedef struct _CO_Mailbox   *Coroutine_Mailbox;     // 邮箱
typedef struct _CO_ASync     *Coroutine_ASync;       // 异步任务
typedef struct _CO_Mutex     *Coroutine_Mutex;       // 互斥锁(可递归)
typedef struct _CO_Channel   *Coroutine_Channel;     // 管道(！！！不能在协程以外的地方使用！！！)

typedef enum
{
    CO_ERR_WATCHDOG_TIMEOUT = 0,   // 看门狗超时
    CO_ERR_STACK_OVERFLOW   = 1,   // 栈溢出
    CO_ERR_MUTEX_RELIEVE    = 2,   // 互斥锁释放错误
    CO_ERR_MEMORY_ALLOC     = 3,   // 内存分配失败
} Coroutine_ErrEvent_t;

/**
 * @brief    错误事件参数
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-07-10
 */
typedef union
{
    // CO_ERR_WATCHDOG_TIMEOUT
    struct
    {
        Coroutine_TaskId taskId;
    } watchdog_timeout;
    // CO_ERR_STACK_OVERFLOW
    struct
    {
        Coroutine_TaskId taskId;
        size_t           stack_size;   // 超出栈大小
    } stack_overflow;
    // CO_ERR_MUTEX_RELIEVE
    struct
    {
        Coroutine_TaskId taskId;
        Coroutine_Mutex  mutex;   // 互斥锁
    } mutex_relieve;
    // CO_ERR_MEMORY_ALLOC
    struct
    {
        int         line;   // 错误发生行
        const char *file;   // 错误发生文件
        size_t      size;   // 内存大小
    } memory_alloc;
} Coroutine_ErrPars_t;

// 任务回调
typedef void (*Coroutine_Task)(void *obj);
// 周期回调（用于看门狗）
typedef void (*Coroutine_Period_Event)(void *object);
// 空闲事件（可用于低功耗休眠）
typedef void (*Coroutine_Idle_Event)(uint32_t time, void *object);
// 线程空闲唤醒
typedef void (*Coroutine_Wake_Event)(uint16_t co_id, void *object);
// 异步任务
typedef void *(*Coroutine_AsyncTask)(void *arg);
// 错误事件
typedef void (*Coroutine_Error_Event)(void                      *object, /* 用户对象 */
                                      int                        line,   /* 事件发生行 */
                                      Coroutine_ErrEvent_t       event,  /* 事件类型 */
                                      const Coroutine_ErrPars_t *pars    /* 事件参数 */
);

/**
 * @brief    协程事件
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-16
 */
typedef struct
{
    void                  *object;   // 用户对象
    Coroutine_Period_Event Period;   // 周期事件
    Coroutine_Idle_Event   Idle;     // 空闲事件
    Coroutine_Wake_Event   wake;     // 唤醒事件
    Coroutine_Error_Event  error;    // 错误事件
} Coroutine_Events;

// 协程接口
typedef struct
{
    size_t thread_count;   // 线程数量

    /**
     * @brief    进入临界保护
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-06-26
     */
    void (*EnterCriticalSection)(const char *file, int line);

    /**
     * @brief    推出临界保护
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-06-26
     */
    void (*LeaveCriticalSection)(const char *file, int line);

    /**
     * @brief    内存分配
     * @param    size           内存分配大小，字节
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-06-26
     */
    void *(*Malloc)(size_t size, const char *file, int line);

    /**
     * @brief    释放内存
     * @param    ptr            内存指针
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-06-26
     */
    void (*Free)(void *ptr, const char *file, int line);

    /**
     * @brief    获取运行毫秒值
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-06-26
     */
    uint64_t (*GetMillisecond)(void);

    /**
     * @brief    获取线程id
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-06-28
     */
    size_t (*GetThreadId)(void);

    /**
     * @brief    事件
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-06-28
     */
    Coroutine_Events *events;
} Coroutine_Inter;

typedef struct
{
    uint64_t id;     // 邮件id
    uint64_t data;   // 邮件数据
    uint32_t size;   // 邮件长度
    bool     isOk;   // 获取成功
} Coroutine_MailResult;

/**
 * @brief    任务属性
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-07-03
 */
typedef struct
{
    int      co_idx;       // 绑定协程索引 小于SetInter设置的线程数量 -1: 随机分配 默认：-1
    uint8_t  pri;          // 优先级  TASK_PRI_LOWEST ~ TASK_PRI_HIGHEST 默认：TASK_PRI_NORMAL
    uint32_t stack_size;   // 栈大小 字节 0：使用默认 根据实际平台分配
} Coroutine_TaskAttribute;

typedef struct
{
    /**
     * @brief    【外部使用】设置接口 ！！！ 必须第一个调头
     * @param    inter          外部接口
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-06-26
     */
    void (*SetInter)(const Coroutine_Inter *inter);

    /**
     * @brief    添加协程任务
     * @param    func           执行函数
     * @param    pars           执行参数
     * @param    name           协程名称
     * @param    attr           任务属性 nullptr:默认属性
     * @return   int            协程id NULL：创建失败
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2022-08-15
     */
    Coroutine_TaskId (*AddTask)(Coroutine_Task                 func,
                                void                          *pars,
                                const char                    *name,
                                const Coroutine_TaskAttribute *attr);

    /**
     * @brief    获取当前任务id
     * @return   Coroutine_TaskId
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2022-08-16
     */
    Coroutine_TaskId (*GetCurrentTaskId)(void);

    /**
     * @brief    获取当前协程索引 从0开始递增 返回 0xFFFF 表示没有当前协程
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-06-28
     */
    uint16_t (*GetCurrentCoroutineIdx)(void);

    /**
     * @brief    获取协程的线程id
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-06-28
     */
    size_t (*GetThreadId)(uint16_t co_id);

    /**
     * @brief    【内部使用】转交控制权
     * @param    coroutine      线程实例
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2022-08-15
     */
    void (*Yield)(void);

    /**
     * @brief    【内部使用】转交控制权
     * @param    coroutine      线程实例
     * @param    timeout        超时 0：不超时
     * @return   uint32_t       多耗时的部分
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2022-08-15
     */
    uint32_t (*YieldDelay)(uint32_t timeout);

    /**
     * @brief    【外部使用】运行协程(一次运行一个任务)
     * @param    c              协程实例
     * @return   true           后续还有任务在运行
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2022-08-17
     */
    bool (*RunTick)(void);

#if COROUTINE_ENABLE_MAILBOX
    /**
     * @brief    创建邮箱
     * @param    name                邮箱名称 最大31字节
     * @param    msg_max_size        邮箱信息最大长度
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-06-26
     */
    Coroutine_Mailbox (*CreateMailbox)(const char *name, uint32_t msg_max_size);

    /**
     * @brief    删除邮箱
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-06-26
     */
    void (*DeleteMailbox)(Coroutine_Mailbox mb);

    /**
     * @brief    发送邮件【不会阻塞】
     * @param    mb             邮箱
     * @param    id             邮件id
     * @param    data           邮件消息
     * @param    size           邮件信息长度
     * @param    timeout        邮件超时
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-06-26
     */
    bool (*SendMail)(Coroutine_Mailbox mb,
                     uint64_t          id,
                     uint64_t          data,
                     uint32_t          size,
                     uint32_t          timeout);

    /**
     * @brief    【内部使用】接收邮件
     * @param    mb             邮箱
     * @param    id_Mask        邮件id掩码
     * @param    timeout        接收超时
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-06-26
     */
    Coroutine_MailResult (*ReceiveMail)(Coroutine_Mailbox mb,
                                        uint64_t          id_Mask,
                                        uint32_t          timeout);
#endif

#if COROUTINE_ENABLE_PRINT_INFO
    /**
     * @brief    显示协程信息
     * @param    buf            缓存
     * @param    max_size       缓存大小
     * @return   int            实际大小
     * @note
     *  SN   TaskId   Func    Pri                 Status Stack                Runtime       WaitTime   DogTime    Name
     * 序号  任务id   函数地址 当前优先级|初始优先级 状态 栈大小/栈最大/栈分配 运行时间(ms) 等待时间(ms) 看门狗时间(ms) 名称
     *   1 00C91124 007D115E  2|2                 MW   1128/1128/16384      14(51%)       58         29958      Task3
     * Status：RUN: 正在运行 SLR: 休眠/就绪 MAI: 等待邮件 SEM: 等待信号 MUT: 等待互斥 CHL: 等待通道 DEL: 死亡
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2022-08-16
     */
    int (*PrintInfo)(char *buf, int max_size);
#endif

#if COROUTINE_ENABLE_SEMAPHORE
    /**
     * @brief    创建信号量
     * @param    name           名称 最大31字节
     * @param    init_val       初始值
     * @return   Coroutine_Semaphore
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2022-08-17
     */
    Coroutine_Semaphore (*CreateSemaphore)(const char *name, uint32_t init_val);

    /**
     * @brief    删除信号量
     * @param    c              协程实例
     * @param    _sem           信号量
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2022-08-17
     */
    void (*DeleteSemaphore)(Coroutine_Semaphore _sem);

    /**
     * @brief    给予信号
     * @param    _sem           信号量
     * @param    val            值
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2022-08-17
     */
    void (*GiveSemaphore)(Coroutine_Semaphore _sem, uint32_t val);

    /**
     * @brief    【内部使用】等待信号量
     * @param    _sem           信号量
     * @param    val            数值
     * @param    timeout        超时
     * @return   true
     * @return   false
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2022-08-17
     */
    bool (*WaitSemaphore)(Coroutine_Semaphore _sem,
                          uint32_t            val,
                          uint32_t            timeout);
#endif

#if COROUTINE_ENABLE_MUTEX
    /**
     * @brief    创建互斥锁
     * @param    name           名称 最大31字节
     * @return   Coroutine_Mutex
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-06-29
     */
    Coroutine_Mutex (*CreateMutex)(const char *name);

    /**
     * @brief    删除互斥锁
     * @param    mutex          互斥锁
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-06-29
     */
    void (*DeleteMutex)(Coroutine_Mutex mutex);

    /**
     * @brief    获取互斥锁
     * @param    mutex          互斥锁
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-06-29
     */
    bool (*LockMutex)(Coroutine_Mutex mutex, uint32_t timeout);

    /**
     * @brief    释放互斥锁
     * @param    mutex          互斥锁
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-06-29
     */
    void (*UnlockMutex)(Coroutine_Mutex mutex);
#endif

    /**
     * @brief    获取毫秒值
     * @return   const Coroutine_Events*
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2022-09-19
     */
    uint64_t (*GetMillisecond)(void);

    /**
     * @brief    内存分配
     * @return   const Coroutine_Events*
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2022-09-19
     */
    void *(*Malloc)(size_t      size,
                    const char *file,
                    int         line);

    /**
     * @brief    内存释放
     * @return   const Coroutine_Events*
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2022-09-19
     */
    void (*Free)(void       *ptr,
                 const char *file,
                 int         line);

    /**
     * @brief    获取任务名称
     * @param    taskId
     * @return   const char*
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-06-27
     */
    const char *(*GetTaskName)(Coroutine_TaskId taskId);

#if COROUTINE_ENABLE_ASYNC
    /**
     * @brief    执行异步任务
     * @param    func           执行函数
     * @param    arg            参数
     * @param    stacks         栈大小 字节 0：使用默认
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-06-28
     */
    Coroutine_ASync (*Async)(Coroutine_AsyncTask func, void *arg, uint32_t stacks);

    /**
     * @brief    等待异步任务完成
     * @param    async          异步任务
     * @param    timeout        等待超时
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-06-28
     */
    bool (*AsyncWait)(Coroutine_ASync async, uint32_t timeout);

    /**
     * @brief    获取异步任务结果，并删除异步任务
     * @param    async_ptr      异步任务指针
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-06-28
     */
    void *(*AsyncGetResultAndDelete)(Coroutine_ASync async);
#endif

#if COROUTINE_ENABLE_SEMAPHORE && COROUTINE_ENABLE_MUTEX && COROUTINE_ENABLE_ASYNC && COROUTINE_ENABLE_WATCHDOG
    /**
     * @brief    获取通用接口
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-07-01
     */
    const Universal *(*GetUniversal)(void);
#endif

#if COROUTINE_ENABLE_WATCHDOG
    /**
     * @brief    喂狗
     * @param    timeout        喂狗超时 0：禁用看门狗
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-07-01
     */
    void (*FeedDog)(uint32_t timeout);
#endif

    /**
     * @brief    设置默认堆栈大小
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-07-05
     */
    void (*SetDefaultStackSize)(uint32_t size);

#if COROUTINE_ENABLE_CHANNEL
    /**
     * @brief    创建通道
     * @param    name           名称 最大31字节
     * @param    caches         缓存数量 0：不缓存
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-07-10
     */
    Coroutine_Channel (*CreateChannel)(const char *name, uint32_t caches);

    /**
     * @brief    删除通道
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-07-10
     */
    void (*DeleteChannel)(Coroutine_Channel ch);

    /**
     * @brief    写通道数据(！！！不能在协程以外的地方使用！！！)
     * @param    ch             通道实例
     * @param    data           写入数据 缓存用完会阻塞
     * @param    timeout        写入超时
     * @return   true           写入成功
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-07-10
     */
    bool (*WriteChannel)(Coroutine_Channel ch, uint64_t data, uint32_t timeout);

    /**
     * @brief    读取通道数据(！！！不能在协程以外的地方使用！！！)
     * @param    ch             通道实例
     * @param    data           读取数据缓存
     * @param    timeout        读取超时
     * @return   true           读取成功
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-07-10
     */
    bool (*ReadChannel)(Coroutine_Channel ch, uint64_t *data, uint32_t timeout);
#endif
} _Coroutine;

/**
 * @brief    协程操作
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-16
 */
extern const _Coroutine Coroutine;
#ifdef __cplusplus
}
#endif
#endif   // !__Coroutine_H__
