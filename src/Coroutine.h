/**
 * @file     Coroutine.h
 * @brief    通用协程
 * @author   CXS (chenxiangshu@outlook.com)
 * @version  1.14
 * @date     2022-08-15
 *
 * @copyright Copyright (c) 2022  Four-Faith
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
 * </table>
 *
 * @note

Coroutine_Handle coroutine = CoroutineEx.Create(inter,false);
CoroutineEx.SetEvent(coroutine, &events);
// 添加任务
CoroutineEx.AddTask(coroutine, test5_task, NULL, 0);
CoroutineEx.AddTask(coroutine, test6_task, NULL, 0);
CoroutineEx.AddTask(coroutine, test7_task, NULL, 0);
// 等待执行完成
while (CoroutineEx.RunTick(coroutine))
    ;
// 删除协程
CoroutineEx.Delete(coroutine);

!!!!!!!!!!!!! 注意 !!!!!!!!!!!!!
1. 禁止将局部变量跨任务使用，因为是共享栈，切换不同任务就会改变栈上的内容
2. 不要在栈上分配一个很大的局部变量，这个会导致任务切换时间变长
*/
#ifndef __Coroutine_H__
#define __Coroutine_H__
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "NodeLink.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CO_Thread *   Coroutine_Handle;      // 协程实例
typedef struct _CO_TCB *      Coroutine_TaskId;      // 任务id
typedef struct _CO_Semaphore *Coroutine_Semaphore;   // 信号量
typedef struct _CO_Mailbox *  Coroutine_Mailbox;     // 邮箱
typedef struct _CO_ASync *    Coroutine_ASync;       // 异步任务
typedef struct _CO_Mutex *    Coroutine_Mutex;       // 互斥锁(可递归)
// 任务回调
typedef void (*Coroutine_Task)(void *obj);
// 周期回调（用于看门狗）
typedef void (*Coroutine_Period_Event)(void *object);
// 内存分配失败
typedef void (*Coroutine_Allocation_Event)(int line, size_t size, void *object);
// 空闲事件（可用于低功耗休眠）
typedef void (*Coroutine_Idle_Event)(uint32_t time, void *object);
// 空闲唤醒
typedef void (*Coroutine_Wake_Event)(void *object);
// 异步任务
typedef void *(*Coroutine_AsyncTask)(void *arg);

/**
 * @brief    协程事件
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-16
 */
typedef struct
{
    void *                     object;       // 用户对象
    Coroutine_Period_Event     Period;       // 周期事件
    Coroutine_Allocation_Event Allocation;   // 分配失败事件
    Coroutine_Idle_Event       Idle;         // 空闲事件
    Coroutine_Wake_Event       wake;         // 唤醒事件
} Coroutine_Events;

// 协程接口
typedef struct
{
    size_t thread_count;   // 线程数量

    /**
    * @brief    加锁，临界保护
    * @author   CXS (chenxiangshu@outlook.com)
    * @date     2024-06-26
    */
    void (*Lock)(const char *file, int line);

    /**
    * @brief    解锁，临界保护
    * @author   CXS (chenxiangshu@outlook.com)
    * @date     2024-06-26
    */
    void (*Unlock)(const char *file, int line);

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

/**
 * @brief    邮件数据
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-16
 */
typedef struct _C_MailData
{
    uint64_t      expiration_time;   // 过期时间
    uint64_t      eventId;           // 事件id
    size_t        size;              // 消息长度
    void *        data;              // 消息数据
    CM_NodeLink_t link;              // _C_MailData
} Coroutine_MailData;

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
   * @param    co_idx         协程索引 小于SetInter设置的线程数量 -1: 随机分配
   * @param    func           执行函数
   * @param    pars           执行参数
   * @param    name           任务名称 最大31字节
   * @return   int            协程id NULL：创建失败
   * @author   CXS (chenxiangshu@outlook.com)
   * @date     2022-08-15
   */
    Coroutine_TaskId (*AddTask)(int co_idx, Coroutine_Task func, void *pars, const char *name);

    /**
   * @brief    获取当前任务id
   * @return   Coroutine_TaskId
   * @author   CXS (chenxiangshu@outlook.com)
   * @date     2022-08-16
   */
    Coroutine_TaskId (*GetCurrentTaskId)(void);

    /**
     * @brief    获取当前协程索引
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
   * @author   CXS (chenxiangshu@outlook.com)
   * @date     2022-08-15
   */
    void (*YieldDelay)(uint32_t timeout);

    /**
     * @brief    【外部使用】运行协程(一次运行一个任务)
     * @param    c              协程实例
     * @return   true           有任务在运行
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2022-08-17
     */
    bool (*RunTick)(void);

    /**
   * @brief    制作消息
   * @param    eventId        事件id
   * @param    data           消息数据
   * @param    size           数据长度
   * @param    time           有效时间
   * @return   MsgData*
   * @author   CXS (chenxiangshu@outlook.com)
   * @date     2022-08-16
   */
    Coroutine_MailData *(*MakeMessage)(uint64_t eventId, const void *data, size_t size, uint32_t time);

    /**
   * @brief   删除消息(发送失败需要删除消息)
   * @param    dat            消息数据
   * @author   CXS (chenxiangshu@outlook.com)
   * @date     2022-08-16
   */
    void (*DeleteMessage)(Coroutine_MailData *dat);

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
    * @brief    发送邮件
    * @param    mb             邮箱
    * @param    data           邮件消息
    * @author   CXS (chenxiangshu@outlook.com)
    * @date     2024-06-26
    */
    bool (*SendMail)(Coroutine_Mailbox mb, Coroutine_MailData *data);

    /**
    * @brief    【内部使用】接收邮件
    * @param    mb             邮箱
    * @param    eventId_Mask   消息id掩码
    * @param    timeout        接收超时
    * @author   CXS (chenxiangshu@outlook.com)
    * @date     2024-06-26
    */
    Coroutine_MailData *(*ReceiveMail)(Coroutine_Mailbox mb,
                                       uint64_t          eventId_Mask,
                                       uint32_t          timeout);

    /**
   * @brief    显示协程信息
   * @param    buf            缓存
   * @param    max_size       缓存大小
   * @return   int            实际大小
   * @author   CXS (chenxiangshu@outlook.com)
   * @date     2022-08-16
   */
    int (*PrintInfo)(char *buf, int max_size);

    /**
   * @brief    【内部使用】结束任务(不能再协程外使用)
   * @param    taskId         任务id
   * @author   CXS (chenxiangshu@outlook.com)
   * @date     2022-08-16
   */
    void (*ExitTask)(Coroutine_TaskId taskId);

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
    void (*Free)(void *      ptr,
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

    /**
     * @brief    执行异步任务
     * @param    func           执行函数
     * @param    _sem           参数
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-06-28
     */
    Coroutine_ASync (*ASync)(Coroutine_AsyncTask func, void *arg);

    /**
     * @brief    等待异步任务完成
     * @param    async          异步任务
     * @param    timeout        等待超时
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-06-28
     */
    bool (*ASyncWait)(Coroutine_ASync async, uint32_t timeout);

    /**
     * @brief    获取异步任务结果，并删除异步任务
     * @param    async_ptr      异步任务指针
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2024-06-28
     */
    void *(*ASyncGetResultAndDelete)(Coroutine_ASync *async_ptr);
} _Coroutine;

/**
 * @brief    协程内部接口（线程不安全）
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-16
 */
extern const _Coroutine Coroutine;

#ifdef __cplusplus
}
#endif
#endif   // !__Coroutine_H__
