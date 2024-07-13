
#include "Coroutine.h"
#include "Coroutine.Platform.h"
#include "RBTree.h"
#include "NodeLink.h"

typedef int STACK_TYPE;   // 栈类型

#define STACK_SENTRY_START 0x5A5A5A5A   // 栈哨兵
#define STACK_SENTRY_END   0xA5A5A5A5   // 栈哨兵

#define MAX_PRIORITY_NUM 5   // 最大优先级数

typedef int (*func_setjmp_t)(jmp_buf buf);
typedef void (*func_longjmp_t)(jmp_buf buf, int val);

// --------------------------------------------------------------------------------------
//                              |       应用        |
// --------------------------------------------------------------------------------------

typedef struct _CO_TCB               CO_TCB;            // 任务控制块/任务节点
typedef struct _CO_Thread            CO_Thread;         // 协程线程
typedef struct _CMessage             CO_Message;        // 消息
typedef struct _CO_Semaphore         CO_Semaphore;      // 信号量
typedef struct _SemaphoreNode        SemaphoreNode;     // 信号节点
typedef struct _MailWaitNode         MailWaitNode;      // 信号节点
typedef struct _CO_Mutex_Wait_Node   MutexWaitNode;     // 互斥锁等待节点
typedef struct _CO_Watchdog_Node     WatchdogNode;      // 看门狗节点
typedef struct _CO_Mailbox           CO_Mailbox;        // 邮箱
typedef struct _CO_ASync             CO_ASync;          // 异步任务
typedef struct _CO_Mutex             CO_Mutex;          // 互斥锁
typedef struct _CO_Channel           CO_Channel;        // 管道
typedef struct _CO_Channel_Wait_Node ChannelWaitNode;   // 管道等待节点
typedef struct _CO_Channel_Data_Node ChannelDataNode;   // 管道数据节点
#if COROUTINE_BLOCK_CRITICAL_SECTION
typedef volatile size_t CO_CS;   // 临界区
#else
typedef volatile char CO_CS;   // 临界区
#endif

/**
 * @brief    信号节点
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-17
 */
struct _SemaphoreNode
{
    bool          isOk;     // 等待成功
    CO_TCB *      task;     // 等待任务
    uint32_t      number;   // 等待数量
    CM_NodeLink_t link;     // _SemaphoreNode
    CO_Semaphore *semaphore;
};

/**
 * @brief    邮件数据
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-16
 */
typedef struct _C_MailData
{
    uint64_t      expiration_time;   // 过期时间
    uint64_t      eventId;           // 事件id
    uint64_t      data;              // 邮件数据
    uint32_t      size;              // 邮件大小
    CM_NodeLink_t link;              // _C_MailData
} Coroutine_MailData;

/**
 * @brief    信号节点
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-17
 */
struct _MailWaitNode
{
    CO_TCB *            task;      // 等待任务
    uint64_t            id_mask;   // id掩码
    CM_NodeLink_t       link;      // _MailWaitNode
    Coroutine_MailData *data;      // 消息数据
    CO_Mailbox *        mailbox;
};

/**
 * @brief    信号量
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-17
 */
struct _CO_Semaphore
{
    char              name[32];     // 名称
    CM_NodeLinkList_t list;         // 等待列表 SemaphoreNode
    uint32_t          value;        // 信号值
    uint32_t          wait_count;   // 等待数
    CM_NodeLink_t     link;         // _CO_Semaphore
    CO_CS cs;   // 临界区
};

struct _CO_Mutex
{
    char              name[32];
    CM_NodeLinkList_t list;            // 等待列表 MutexWaitNode
    uint32_t          value;           // 互斥值
    uint32_t          wait_count;      // 等待数量
    uint32_t          max_wait_time;   // 最大等待时间
    CM_NodeLink_t     link;            // _CO_Mutex
    CO_TCB *          owner;           // 持有者
    CO_CS cs;   // 临界区
};

struct _CO_Mutex_Wait_Node
{
    CM_NodeLink_t link;    // MutexWaitNode
    CO_Mutex *    mutex;   // 等待互斥锁
    uint64_t      time;    // 等待时间
    CO_TCB *      task;    // 等待任务
};

struct _CO_Watchdog_Node
{
    CM_RBTree_Link_t link;              // WatchdogNode
    uint64_t         expiration_time;   // 看门狗过期时间 0：不启用
    CO_TCB *         task;              // 等待任务
};

/**
 * @brief    协程节点
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-15
 */
struct _CO_TCB
{
    jmp_buf        env;                          // 环境
    uint64_t       execv_time;                   // 执行时间
    uint32_t       stack_len;                    // 当前栈长度
    uint32_t       stack_max;                    // 最大长度
    uint32_t       stack_alloc;                  // 分配长度
    uint16_t       isDel : 1;                    // 是否删除
    uint16_t       isRun : 1;                    // 正在运行
    uint16_t       isWaitMail : 1;               // 正在等待邮件
    uint16_t       isWaitSem : 1;                // 等待信号量
    uint16_t       isWaitMutex : 1;              // 等待互斥量
    uint16_t       isWaitChannel : 1;            // 等待管道
    uint16_t       isFirst : 1;                  // 首次
    uint16_t       isAddRunList : 1;             // 添加运行列表
    uint16_t       isAddRunStandaloneList : 1;   // 添加运行列表
    uint16_t       isAddSleepList : 1;           // 添加睡眠列表
    uint16_t       isAddStopList : 1;            // 添加停止列表
    uint16_t       isDynamicThread : 1;          // 动态线程
    uint16_t       isPrint : 1;                  // 已打印
    uint16_t       isRuning : 1;                 // 正在运行
    uint8_t        priority;                     // 当前优先级
    uint8_t        init_priority;                // 初始化优先级
    Coroutine_Task func;                         // 执行
    char *         name;                         // 名称
    void *         obj;                          // 执行参数
    CO_Thread *    coroutine;                    // 父节点
    STACK_TYPE *   stack;                        // 栈缓存
#if COROUTINE_ENABLE_PRINT_INFO
    uint64_t run_time;   // 运行时间
#endif

    WatchdogNode *watchdog;   // 看门狗节点

    CM_NodeLink_t    run_stop_link;    // _CO_TCB , 运行节点
    CM_NodeLink_t    task_list_link;   // 任务列表节点
    CM_RBTree_Link_t sleep_link;       // 睡眠节点
};

/**
 * @brief    协程线程
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-15
 */
struct _CO_Thread
{
    jmp_buf              env;               // 环境
    uint8_t              isRun : 1;         // 运行
    uint8_t              isSleep : 1;       // 正在睡眠
    uint8_t              _run_task_cnt;     // 运行任务次数
    volatile STACK_TYPE *stack;             // 栈
    CO_TCB *             idx_task;          // 当前任务
    size_t               ThreadId;          // 当前协程的线程id
    uint16_t             co_id;             // 协程id
    uint64_t             task_start_time;   // 任务开始时间

#if COROUTINE_ENABLE_PRINT_INFO
    uint64_t start_time;   // 启动时间
    uint64_t sleep_time;   // 休眠时间
#endif

    CM_NodeLinkList_t tasks_run[MAX_PRIORITY_NUM];   // 运行任务列表 CO_TCB，根据优先级
    CM_NodeLinkList_t tasks_stop;                    // 停止列表 CO_TCB
    CM_RBTree_t       tasks_sleep;                   // 睡眠任务列表 CO_TCB
    CO_TCB *          idx_sleep;                     // 当前休眠任务

    CM_NodeLink_t link;   // _CO_Thread
};

/**
 * @brief    
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-06-26
 */
struct _CO_Mailbox
{
    char              name[32];     // 名称
    uint32_t          total;        // 总数
    uint32_t          size;         // 邮箱大小
    uint32_t          wait_count;   // 等待数量
    uint32_t          mail_count;   // 邮件数量
    CM_NodeLinkList_t mails;        // 邮件列表
    CM_NodeLinkList_t waits;        // 等待列表 MailWaitNode
    CM_NodeLink_t     link;         // _CO_Mailbox
    CO_CS cs;   // 临界区
};

struct _CO_ASync
{
    Coroutine_AsyncTask func;
    void *              object;
    void *              ret;
    Coroutine_Semaphore sem;
};

struct _CO_Channel_Wait_Node
{
    bool          isOk;   // 等待成功
    uint64_t      data;   // 数据
    CO_TCB *      task;   // 等待任务
    CM_NodeLink_t link;   // ChannelWaitNode
};

struct _CO_Channel_Data_Node
{
    uint64_t      data;
    CM_NodeLink_t link;
};

struct _CO_Channel
{
    char              name[32];   // 名称
    uint32_t          size;       // 缓存数量
    CM_NodeLinkList_t waits;      // 等待列表 ChannelWaitNode
    CM_NodeLinkList_t caches;     // 缓存列表 ChannelDataNode
    CM_NodeLinkList_t senders;    // 发送者列表 ChannelWaitNode
    CM_NodeLink_t     link;       // CO_Channel
    CO_CS cs;   // 临界区
};

static Coroutine_Inter Inter;   // 外部接口

static struct
{
    CM_NodeLinkList_t threads;                                  // 协程控制器列表
    CM_NodeLinkList_t semaphores;                               // 信号列表
    CM_NodeLinkList_t mailboxes;                                // 邮箱列表
    CM_NodeLinkList_t mutexes;                                  // 互斥列表 _CO_Mutex
    CM_NodeLinkList_t task_list;                                // 任务列表
    CM_NodeLinkList_t channels;                                 // 管道列表
    CO_Thread **      coroutines;                               // 协程控制器
    CM_RBTree_t       watchdogs;                                // 看门狗列表 WatchdogNode 从小到大
    CO_TCB *          idx_watchdog;                             // 当前看门狗
    uint64_t          check_watchdog_time;                      // 看门狗检查时间
    CM_NodeLinkList_t standalone_tasks_run[MAX_PRIORITY_NUM];   // 独立栈 运行任务列表 CO_TCB，根据优先级
    uint32_t          wait_run_standalone_task_count;           // 等待运行任务数量
    uint32_t          def_stack_size;                           // 默认栈大小
    uint32_t          sleep_thread_count;                       // 休眠线程数量
    uint16_t              ThreadAllocNum;                          // 线程分配数量

#if COROUTINE_BLOCK_CRITICAL_SECTION
    CO_CS cs_semaphores;   // 临界区
    CO_CS cs_mailboxes;    // 临界区
    CO_CS cs_mutexes;      // 临界区
    CO_CS cs_watchdogs;    // 临界区
#endif
} C_Static;

#define CO_EnterCriticalSection() Inter.EnterCriticalSection(__FILE__, __LINE__)
#define CO_LeaveCriticalSection() Inter.LeaveCriticalSection(__FILE__, __LINE__);
// 设置任务执行时间
#define CO_SET_TASK_TIME(task, time)                          \
    do {                                                      \
        if (time)                                             \
            task->execv_time = time + Inter.GetMillisecond(); \
        else                                                  \
            task->execv_time = 0;                             \
    } while (false)

#if COROUTINE_ENABLE_SEMAPHORE
static void DeleteMessage(Coroutine_MailData *dat);
#endif
static Coroutine_Handle       Coroutine_Create(size_t id);
static CO_Thread *            GetCurrentThread(int co_idx);
volatile static func_setjmp_t _c_setjmp = setjmp;

#define _ERROR_IDLE                                               \
    while (true) {                                                \
        if (Inter.events->Idle)                                   \
            Inter.events->Idle(UINT32_MAX, Inter.events->object); \
    }

#define _ERROR_CALL(event, pars)                                                                \
    if (Inter.events->error) Inter.events->error(Inter.events->object, __LINE__, event, &pars); \
    _ERROR_IDLE

// 栈错误
#define ERROR_STACK(task, size)                                  \
    do {                                                         \
        Coroutine_ErrPars_t pars;                                \
        pars.stack_overflow.taskId     = (Coroutine_TaskId)task; \
        pars.stack_overflow.stack_size = size;                   \
        _ERROR_CALL(CO_ERR_STACK_OVERFLOW, pars);                \
    } while (false)

// 内存错误
#define ERROR_MEMORY_ALLOC(_file, _line, _size) \
    do {                                        \
        Coroutine_ErrPars_t pars;               \
        pars.memory_alloc.file = _file;         \
        pars.memory_alloc.line = _line;         \
        pars.memory_alloc.size = _size;         \
        _ERROR_CALL(CO_ERR_MEMORY_ALLOC, pars); \
    } while (false)

// 互斥锁释放错误
#define ERROR_MUTEX_RELIEVE(task, _mutex)                   \
    do {                                                    \
        Coroutine_ErrPars_t pars;                           \
        pars.mutex_relieve.taskId = (Coroutine_TaskId)task; \
        pars.mutex_relieve.mutex  = _mutex;                 \
        _ERROR_CALL(CO_ERR_MUTEX_RELIEVE, pars);            \
    } while (false)

// 看门狗超时
#define ERROR_WATCHDOG_TIMEOUT(task)                           \
    do {                                                       \
        Coroutine_ErrPars_t pars;                              \
        pars.watchdog_timeout.taskId = (Coroutine_TaskId)task; \
        _ERROR_CALL(CO_ERR_WATCHDOG_TIMEOUT, pars);            \
    } while (false)

// 信号量删除错误
#define ERROR_SEM_DELETE(_sem)                      \
    do {                                            \
        Coroutine_ErrPars_t pars;                   \
        pars.sem_delete.sem = _sem;                 \
        _ERROR_CALL(CO_ERR_WATCHDOG_TIMEOUT, pars); \
    } while (false)

// CO_ERR_MUTEX_DELETE
#define ERROR_MUTEX_DELETE(_mutex)              \
    do {                                        \
        Coroutine_ErrPars_t pars;               \
        pars.mutex_delete.mutex = _mutex;       \
        _ERROR_CALL(CO_ERR_MUTEX_DELETE, pars); \
    } while (false)

// 检查栈哨兵
#define CHECK_STACK_SENTRY(n)                                                                    \
    if (n->stack[0] != STACK_SENTRY_END || n->stack[n->stack_alloc - 1] != STACK_SENTRY_START) { \
        ERROR_STACK(n, 0);                                                                       \
    }

// --------------------------------------------------------------------------------------
//                              |       内核调度        |
// --------------------------------------------------------------------------------------

#if COROUTINE_CHECK_STACK || COROUTINE_ENABLE_PRINT_INFO
/**
 * @brief    检查任务栈
 * @param    t              
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-07-04
 */
static void CheckStack(volatile CO_TCB *t)
{
    uint32_t *p = (uint32_t *)t->stack;
    CHECK_STACK_SENTRY(t);
    uint32_t idx = 1;
    for (; idx < t->stack_alloc - 1; idx++) {
        if (p[idx] != 0xEEEEEEEE)
            break;
    }
    if (idx == 1) {
        /* 栈错误 */
        ERROR_STACK(t, 0);
    }
    uint32_t s = t->stack_alloc - idx;
    if (s > t->stack_max) t->stack_max = s;
    return;
}
#endif

/**
 * @brief    获取可能休眠的线程
 * @return   * CO_Thread*
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-07-04
 */
static CO_Thread *GetSleepThread(void)
{
    if (Inter.thread_count == 1)
        return C_Static.coroutines[0];
    int idx = rand() % Inter.thread_count;
    for (size_t i = 0; i < Inter.thread_count && C_Static.sleep_thread_count; i++) {
        if (C_Static.coroutines[idx]->isSleep) {
            C_Static.coroutines[idx]->isSleep = 0;
            break;
        }
        idx = (idx + 1) % Inter.thread_count;
    }
    return C_Static.coroutines[idx];
}

/**
 * @brief    删除任务
 * @param    inter
 * @param    t
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-17
 */
static void DeleteTask(CO_TCB *t)
{
    // 移除看门狗
    if (t->watchdog && t->watchdog->expiration_time) {
        // CO_APP_ENTER(&C_Static.cs_watchdogs);
        CM_RBTree_Remove(&C_Static.watchdogs, &t->watchdog->link);
        if (C_Static.idx_watchdog == t) {
            WatchdogNode *n       = CM_Field_ToType(WatchdogNode, link, CM_RBTree_LeftEnd(&C_Static.watchdogs));
            C_Static.idx_watchdog = n == nullptr ? nullptr : n->task;
        }
        // CO_APP_LEAVE(&C_Static.cs_watchdogs);
    }
    if (t->watchdog) Inter.Free(t->watchdog, __FILE__, __LINE__);
    // 移除任务列表
    CM_NodeLink_Remove(&C_Static.task_list, &t->task_list_link);
    // 释放名称
    if (t->name) Inter.Free(t->name, __FILE__, __LINE__);
    // 释放堆栈
    Inter.Free(t->stack, __FILE__, __LINE__);
    // 释放实例
    Inter.Free(t, __FILE__, __LINE__);
    return;
}

/**
 * @brief    加入任务列表 返回唤醒co_id
 * @param    coroutine      
 * @param    task           
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-06-28
 */
static void AddTaskList(CO_TCB *task, uint8_t new_pri)
{
    if (task->isRuning)
        return;
    CO_Thread *coroutine = task->coroutine;
    uint64_t   now       = Inter.GetMillisecond();
    if (task->isAddRunList) {
        CM_NodeLink_Remove(&coroutine->tasks_run[task->priority], &task->run_stop_link);
        task->isAddRunList = 0;
    } else if (task->isAddSleepList) {
        // 从休眠树中移除
        CM_RBTree_Remove(&coroutine->tasks_sleep, &task->sleep_link);
        if (task == coroutine->idx_sleep) {
            // 更新当前休眠任务
            CM_RBTree_Link_t *link = CM_RBTree_LeftEnd(&coroutine->tasks_sleep);
            coroutine->idx_sleep   = link == nullptr ? nullptr : CM_Field_ToType(CO_TCB, sleep_link, link);
        }
        task->isAddSleepList = 0;
    } else if (task->isAddStopList) {
        CM_NodeLink_Remove(&coroutine->tasks_stop, &task->run_stop_link);
        task->isAddStopList = 0;
    } else if (task->isAddRunStandaloneList) {
        CM_NodeLink_Remove(&C_Static.standalone_tasks_run[task->priority], &task->run_stop_link);
        task->isAddRunStandaloneList = 0;
        C_Static.wait_run_standalone_task_count--;
    }
    if (new_pri != task->priority)
        task->priority = new_pri;   // 更新优先级
    if (task->isRun == 0) {         // 加入停止列表
        CM_NodeLink_Insert(&coroutine->tasks_stop,
                           CM_NodeLink_End(coroutine->tasks_stop),
                           &task->run_stop_link);
        task->isAddStopList = 1;
    } else if (task->isDel || now >= task->execv_time) {   // 加入运行列表
        if (task->isDynamicThread == 0 ||
            Inter.thread_count == 1 ||
            !coroutine_is_dynamic_run_thread()) {
            // 共享栈，加入本协程控制器
            CM_NodeLink_Insert(&coroutine->tasks_run[task->priority],
                               CM_NodeLink_End(coroutine->tasks_run[task->priority]),
                               &task->run_stop_link);
            task->isAddRunList = 1;
        } else {
            // 独立栈，加入公共列表
            task->coroutine = nullptr;
            CM_NodeLink_Insert(&C_Static.standalone_tasks_run[task->priority],
                               CM_NodeLink_End(C_Static.standalone_tasks_run[task->priority]),
                               &task->run_stop_link);
            task->isAddRunStandaloneList = 1;
            C_Static.wait_run_standalone_task_count++;
            // 唤醒线程
            if (C_Static.sleep_thread_count && Inter.events->wake)
                Inter.events->wake(GetSleepThread()->co_id, Inter.events->object);
        }
    } else {
        // 加入休眠列表
        CM_RBTree_Insert(&coroutine->tasks_sleep, &task->sleep_link);
        task->isAddSleepList = 1;
        if (coroutine->idx_sleep == nullptr || coroutine->idx_sleep->execv_time > task->execv_time)
            coroutine->idx_sleep = task;   // 当前休眠任务设置为最早的任务
    }
    return;
}

/**
 * @brief    获取休眠任务
 * @param    coroutine      
 * @param    ts             
 * @return   * uint64_t     
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-06-28
 */
static uint32_t GetSleepTask(CO_Thread *coroutine, uint64_t ts)
{
    if (coroutine->idx_sleep == nullptr)
        return 0;
    CO_TCB *task = coroutine->idx_sleep;
    if (ts >= task->execv_time) {
        // 移除休眠列表,加入运行列表
        AddTaskList(task, task->priority);
        return 0;
    }
    return task->execv_time >= ts + UINT32_MAX ? UINT32_MAX : task->execv_time - ts;
}

static CO_TCB *GetNextTask_LocalTask(CO_Thread *coroutine, uint8_t pri)
{
    CO_TCB *task = nullptr;
    if (!CM_NodeLink_IsEmpty(coroutine->tasks_run[pri])) {
        task = CM_Field_ToType(CO_TCB, run_stop_link, CM_NodeLink_First(coroutine->tasks_run[pri]));
        // 移除运行列表
        CM_NodeLink_Remove(&coroutine->tasks_run[pri], &task->run_stop_link);
        task->isAddRunList = 0;
    }
    return task;
}

static CO_TCB *GetNextTask_StandaloneTask(CO_Thread *coroutine, uint8_t pri)
{
    CO_TCB *task = nullptr;
    if (!CM_NodeLink_IsEmpty(C_Static.standalone_tasks_run[pri])) {
        task = CM_Field_ToType(CO_TCB, run_stop_link, CM_NodeLink_First(C_Static.standalone_tasks_run[pri]));
        // 移除运行列表
        CM_NodeLink_Remove(&C_Static.standalone_tasks_run[pri], &task->run_stop_link);
        task->isAddRunStandaloneList = 0;
        C_Static.wait_run_standalone_task_count--;
    }
    return task;
}

/**
 * @brief    获取下一个任务
 * @param    coroutine      协程实例
 * @return   CO_Thread*
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-15
 */
static CO_TCB *GetNextTask(CO_Thread *coroutine)
{
    if (coroutine->isRun == 0)
        return nullptr;
    CO_TCB *task    = nullptr;
    bool    isLocal = coroutine->_run_task_cnt & 0x01;
    for (int i = 0; i < MAX_PRIORITY_NUM && task == nullptr; i++) {
        if (isLocal) {
            // 本地优先
            task = GetNextTask_LocalTask(coroutine, i);
            if (task == nullptr)
                task = GetNextTask_StandaloneTask(coroutine, i);
        } else {
            // 外部优先
            task = GetNextTask_StandaloneTask(coroutine, i);
            if (task == nullptr)
                task = GetNextTask_LocalTask(coroutine, i);
        }
    }
    if (task == nullptr)
        return nullptr;
    // 检查任务是否需要删除
    if (task->isDel) {
        DeleteTask(task);
        return nullptr;
    } else if (task->isRun == 0) {
        // 加入停止列表
        AddTaskList(task, task->priority);
        return nullptr;
    }
    task->coroutine     = coroutine;
    task->isRuning      = 1;   // 设置运行标志
    coroutine->idx_task = task;
    coroutine->_run_task_cnt++;
    return task;
}

/**
 * @brief    检查看门狗
 * @param    coroutine      
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-07-01
 */
static void CheckWatchdog(void)
{
    if (Inter.GetMillisecond() < C_Static.check_watchdog_time || C_Static.idx_watchdog == nullptr)
        return;
    C_Static.check_watchdog_time = Inter.GetMillisecond() + 10;   // 每 10ms 检查一次
    CO_TCB *t                    = C_Static.idx_watchdog;
    if (t && t->watchdog && t->watchdog->expiration_time < Inter.GetMillisecond()) {
        // 看门狗超时
        // ERROR_WATCHDOG_TIMEOUT(t);
        C_Static.check_watchdog_time = 0;
    }
    return;
}

static void __task(CO_TCB *n)
{
    // 执行任务
    n->func(n->obj);
    // 任务结束
    n->priority = 0;
    n->isDel    = true;
    // 回到调度器
    longjmp(n->coroutine->env, 1);
}

static void _enter_into(volatile CO_TCB *n)
{
    volatile STACK_TYPE __stack = 0x44332211;
    // 获取栈起始指针
    if (n->coroutine->stack == nullptr)
        n->coroutine->stack = (STACK_TYPE *)&__stack;   // 不能在后面创建局部变量
    else if (n->coroutine->stack != (STACK_TYPE *)&__stack)
        return;   // 栈错误,栈被迁移
    // 保存环境
    int ret = _c_setjmp(n->coroutine->env);
    if (ret == 0) {
        if (n->isFirst) {
            // 设置已运行标志
            n->isFirst = false;
            // 第一次运行
            // 栈对齐
            STACK_TYPE *stack = nullptr;
            if (coroutine_get_stack_direction()) {
                // 栈向上增长
                stack = (STACK_TYPE *)(n->stack + 1);
                while ((size_t)stack % sizeof(void *))
                    stack++;
            } else {
                // 栈向下增长
                stack = (STACK_TYPE *)(n->stack + n->stack_alloc - 1);
                while ((size_t)stack % sizeof(void *))
                    stack--;
            }
            // 使用新的栈进入函数
            coroutine_enter_task(__task, (void *)n, stack);
            //! 不会执行到这里
            ERROR_STACK(n, 0);
        } else {
            // 回到 SaveStack.setjmp
            longjmp(((CO_TCB *)n)->env, 1);
        }
    }
    return;
}

/**
 * @brief    协程任务
 * @param    obj
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-15
 */
static void _Task(CO_Thread *coroutine)
{
    CO_TCB * n                 = nullptr;
    uint32_t sleep_ms          = UINT32_MAX;
    coroutine->task_start_time = Inter.GetMillisecond();
    // 获取下一个任务
    CheckWatchdog();
    CO_EnterCriticalSection();
    sleep_ms = GetSleepTask(coroutine, coroutine->task_start_time);
    n        = GetNextTask(coroutine);
    CO_LeaveCriticalSection();
    if (n == nullptr) {
        // 运行空闲任务
        if (sleep_ms > 1 && Inter.events->Idle != nullptr) {
#if COROUTINE_ENABLE_PRINT_INFO
            uint64_t now = Inter.GetMillisecond();
#endif
            CO_EnterCriticalSection();
            coroutine->isSleep = 1;   // 设置休眠状态
            C_Static.sleep_thread_count++;
            CO_LeaveCriticalSection();
            Inter.events->Idle(sleep_ms - 1, Inter.events->object);
            CO_EnterCriticalSection();
            coroutine->isSleep = 0;   // 清除休眠状态
            C_Static.sleep_thread_count--;
            CO_LeaveCriticalSection();
#if COROUTINE_ENABLE_PRINT_INFO
            now = Inter.GetMillisecond() - now;
            coroutine->sleep_time += now;
#endif
        }
        return;
    }
    // 执行任务
    _enter_into(n);
    // 记录运行时间
#if COROUTINE_ENABLE_PRINT_INFO
    uint64_t ts = Inter.GetMillisecond();
    uint64_t tv = ts <= coroutine->task_start_time ? 0 : ts - coroutine->task_start_time;
    if (tv) {
        n->run_time += tv;
    }
#endif
    // 添加到运行列表
    int co_id = -1;
    CO_EnterCriticalSection();
    n->isRuning         = 0;   // 清除运行标志
    coroutine->idx_task = nullptr;
    // 加入任务列表
    AddTaskList((CO_TCB *)n, n->priority);
    if (Inter.thread_count > 1 &&
        n->isAddRunStandaloneList &&
        C_Static.wait_run_standalone_task_count > 1) {
        // 唤醒其他线程
        for (int i = 0; i < Inter.thread_count; i++) {
            if (C_Static.coroutines[i]->isSleep) {
                co_id                           = C_Static.coroutines[i]->co_id;
                C_Static.coroutines[i]->isSleep = 0;
                break;
            }
        }
    }
    CO_LeaveCriticalSection();
    if (co_id >= 0 && Inter.events->wake != nullptr)
        Inter.events->wake(co_id, Inter.events->object);   // 唤醒其他空闲线程
    // 周期事件
    if (Inter.events->Period != nullptr)
        Inter.events->Period(Inter.events->object);
    return;
}

/**
 * @brief    上下文切换【独立栈】
 * @param    n              
 * @return   * void         
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-07-03
 */
static void ContextSwitch(CO_TCB *n, CO_Thread *coroutine)
{
    // 保存环境,回到调度器
    int ret = _c_setjmp(((CO_TCB *)n)->env);
    if (ret == 0) {
        volatile STACK_TYPE __mem   = 0x11223344;                             // 利用局部变量获取堆栈寄存器值
        STACK_TYPE *        p_stack = (STACK_TYPE *)(((size_t)&__mem) - 4);   // 获取栈结尾
        // 检查栈溢出
        if (p_stack <= n->stack) {
            // 栈溢出
            ERROR_STACK(n, (n->stack - p_stack) * sizeof(STACK_TYPE));
        }
        // 计算栈大小
        n->stack_len = n->stack_alloc - (p_stack - n->stack);
        // 记录最大栈大小
        if (n->stack_len > n->stack_max) n->stack_max = n->stack_len;
#if COROUTINE_CHECK_STACK
        CheckStack(n);
#else
        // 检查栈哨兵
        CHECK_STACK_SENTRY(n);
#endif
        // 回到调度器
        longjmp(coroutine->env, 1);
    }
    return;
}

// --------------------------------------------------------------------------------------
//                              |       外部接口        |
// --------------------------------------------------------------------------------------

/**
 * @brief    转交控制权
 * @param    coroutine      线程实例
 * @param    timeout        超时
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-15
 */
static void _Yield(void)
{
    CO_Thread *coroutine = GetCurrentThread(-1);
    if (coroutine == nullptr || coroutine->idx_task == nullptr)
        return;
    // 保存堆栈数据
    ContextSwitch(coroutine->idx_task, coroutine);
    return;
}

#if COROUTINE_BLOCK_CRITICAL_SECTION
#define CO_APP_EnterCriticalSection() CO_EnterCriticalSection()
#define CO_APP_LeaveCriticalSection() CO_LeaveCriticalSection()
#else
#define CO_APP_EnterCriticalSection()
#define CO_APP_LeaveCriticalSection()
#endif

/**
 * @brief    进入临界区(非内核，不可重入)
 * @param    cs             
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-07-12
 */
#if COROUTINE_BLOCK_CRITICAL_SECTION
static void CO_APP_ENTER(CO_CS *cs)
{
    size_t id   = Inter.GetThreadId();
    bool   isOk = false;
    while (!isOk) {
        CO_EnterCriticalSection();
        if (*cs == 0) {
            *cs  = id;
            isOk = true;
        }
        CO_LeaveCriticalSection();
        if (!isOk) {
            _Yield();                   // 等待其他线程释放临界区
            id = Inter.GetThreadId();   // 获取新线程id
        }
    }
    return;
}
#else
#define CO_APP_ENTER(cs) CO_EnterCriticalSection()
#endif

/**
 * @brief    离开临界区(不可重入)
 * @param    cs             
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-07-12
 */
#if COROUTINE_BLOCK_CRITICAL_SECTION
static void CO_APP_LEAVE(CO_CS *cs)
{
    size_t id = Inter.GetThreadId();
    CO_EnterCriticalSection();
    if (*cs == id)
        *cs = 0;
    else {
        while (true)   // Debug
            ;
    }
    CO_LeaveCriticalSection();
    return;
}
#else
#define CO_APP_LEAVE(cs) CO_LeaveCriticalSection()
#endif

/**
 * @brief    获取当前协程控制器
 * @param    co_idx         协程索引
 * @param    c              协程实例
 * @return   CO_Thread*     
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-06-28
 */
static CO_Thread *GetCurrentThread(int co_idx)
{
    CO_Thread *ret = nullptr;
    if (Inter.thread_count == 1) {
        ret = C_Static.coroutines[0];
        if (ret->ThreadId == (size_t)-1)
            ret->ThreadId = Inter.GetThreadId();
    } else {
            if (co_idx < 0) {
                size_t id  = Inter.GetThreadId();
            if(C_Static.ThreadAllocNum == Inter.thread_count)
            {
                size_t s=0,idx = 0,e=Inter.thread_count-1;
                while(s<=e)
                {
                    idx = (s+e)>>1;
                    if(C_Static.coroutines[idx]->ThreadId == id)
                    {
                        ret = C_Static.coroutines[idx];
                        break;
                    }
                    else if(C_Static.coroutines[idx]->ThreadId < id)
                        s = idx+1;
                    else
                        e = idx-1;
                }
            }else {
                // 分配线程
                CO_EnterCriticalSection();
                C_Static.coroutines[C_Static.ThreadAllocNum++]->ThreadId = id;
                if(C_Static.ThreadAllocNum == Inter.thread_count)
                {
                    // 排序
                    for (int i = 0; i < Inter.thread_count;i++)
                    {
                        for (int j = i+1; j < Inter.thread_count ; j++)
                        {
                            if(C_Static.coroutines[j]->ThreadId < C_Static.coroutines[i]->ThreadId)
                            {
                                id = C_Static.coroutines[i]->ThreadId;
                                C_Static.coroutines[i]->ThreadId = C_Static.coroutines[j]->ThreadId;
                                C_Static.coroutines[j]->ThreadId = id;
                            }
                        }
                    }
                }
                CO_LeaveCriticalSection();
                return nullptr;
            }
        } else {
            CM_NodeLink_Foreach_Positive(CO_Thread, link, C_Static.threads, p)
            {
                if (p->co_id == co_idx) {
                    ret = p;
                    break;
                }
            }
        }
    }
    return ret;
}

/**
 * @brief    运行协程(一次运行一个任务)
 * @param    c              协程实例
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-17
 */
static bool Coroutine_RunTick(void)
{
    CO_Thread *coroutine = GetCurrentThread(-1);
    if (coroutine == nullptr)
        return false;
    _Task(coroutine);
    return true;
}

static Coroutine_TaskId AddTask(CO_Thread *    coroutine,
                                Coroutine_Task func,
                                void *         pars,
                                uint8_t        pri,
                                const char *   name,
                                uint32_t       stack_size)
{
    if (func == nullptr)
        return nullptr;
    stack_size = ALIGN(stack_size, sizeof(STACK_TYPE));
    CO_TCB *n  = (CO_TCB *)Inter.Malloc(sizeof(CO_TCB), __FILE__, __LINE__);
    if (n == nullptr) ERROR_MEMORY_ALLOC(__FILE__, __LINE__, sizeof(CO_TCB));
    CM_ZERO(n);
    n->func            = func;
    n->obj             = pars;
    n->stack           = nullptr;
    n->coroutine       = coroutine;
    n->isRun           = true;
    n->isFirst         = true;
    n->isDynamicThread = coroutine == nullptr;
    n->stack           = (STACK_TYPE *)Inter.Malloc(stack_size * sizeof(STACK_TYPE), __FILE__, __LINE__);   // 预分配 512 字节
    n->stack_alloc     = stack_size;
    n->init_priority = n->priority = pri;
    CM_NodeLink_Init(&n->run_stop_link);
    if (name != nullptr && name[0] != '\0') {
        n->name = (char *)Inter.Malloc(32, __FILE__, __LINE__);
        if (n->name == nullptr) ERROR_MEMORY_ALLOC(__FILE__, __LINE__, 32);
        int s = strlen(name);
        if (s > 31) s = 31;
        memcpy(n->name, name, s + 1);
    }
    // 初始化栈空间
    memset(n->stack, 0xEE, n->stack_alloc * sizeof(STACK_TYPE));
    // 设置哨兵
    n->stack[0]                  = STACK_SENTRY_END;
    n->stack[n->stack_alloc - 1] = STACK_SENTRY_START;
    // 添加到任务列表
    CO_EnterCriticalSection();
    CM_NodeLink_Insert(&C_Static.task_list, CM_NodeLink_End(C_Static.task_list), &n->task_list_link);
    AddTaskList(n, n->priority);
    CO_LeaveCriticalSection();
    return n;
}

static Coroutine_TaskId Coroutine_AddTask(Coroutine_Task                 func,
                                          void *                         pars,
                                          const char *                   name,
                                          const Coroutine_TaskAttribute *attr)
{
    CO_Thread *coroutine  = nullptr;
    int        co_idx     = attr == nullptr ? -1 : attr->co_idx;
    uint8_t    pri        = attr == nullptr ? TASK_PRI_NORMAL : attr->pri;
    uint32_t   stack_size = attr == nullptr ? 0 / sizeof(STACK_TYPE) : attr->stack_size;   // 默认 4KB 栈空间
    if (stack_size == 0)
        stack_size = C_Static.def_stack_size;
    if (Inter.thread_count == 1)
        coroutine = C_Static.coroutines[0];
    else if (co_idx >= 0)
        coroutine = GetCurrentThread(co_idx);
    return AddTask(coroutine, func, pars, pri, name, stack_size);
}

/**
 * @brief    获取当前任务id
 * @param    c              协程实例
 * @return   Coroutine_TaskId
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-16
 */
static Coroutine_TaskId Coroutine_GetTaskId(void)
{
    CO_Thread *coroutine = GetCurrentThread(-1);
    return coroutine == nullptr ? nullptr : coroutine->idx_task;
}

static uint16_t GetCurrentCoroutineIdx(void)
{
    CO_Thread *coroutine = GetCurrentThread(-1);
    return coroutine == nullptr ? -1 : coroutine->co_id;
}

static size_t GetThreadId(uint16_t co_id)
{
    if (co_id >= Inter.thread_count)
        return (size_t)-1;
    CO_Thread *coroutine = GetCurrentThread(co_id);
    return coroutine == nullptr ? (size_t)-1 : coroutine->ThreadId;
}

/**
 * @brief    转交控制权
 * @param    coroutine      协程实例
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-15
 */
static void Coroutine_Yield(void)
{
    _Yield();
    return;
}

/**
 * @brief    转交控制权
 * @param    timeout        超时 0：不超时
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-15
 */
static uint32_t Coroutine_YieldTimeOut(uint32_t timeout)
{
    if (timeout == 0) {
        _Yield();
        return 0;
    }
    uint64_t   ts        = Inter.GetMillisecond();
    CO_Thread *coroutine = GetCurrentThread(-1);
    // 设置超时时间
    CO_EnterCriticalSection();
    CO_SET_TASK_TIME(coroutine->idx_task, timeout);
    CO_LeaveCriticalSection();
    // 转交控制权
    _Yield();
    ts = Inter.GetMillisecond() - ts;                   // 计算耗时
    return ts > timeout ? (uint32_t)ts - timeout : 0;   // 返回误差
}

// --------------------------------------------------------------------------------------
//                              |       邮箱通信        |
// --------------------------------------------------------------------------------------

#if COROUTINE_ENABLE_MAILBOX
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
static Coroutine_MailData *MakeMessage(uint64_t eventId,
                                       uint64_t data,
                                       uint32_t size,
                                       uint32_t time)
{
    Coroutine_MailData *dat = (Coroutine_MailData *)Inter.Malloc(sizeof(Coroutine_MailData), __FILE__, __LINE__);
    if (dat == nullptr) ERROR_MEMORY_ALLOC(__FILE__, __LINE__, sizeof(Coroutine_MailData));
    dat->data            = data;
    dat->size            = size;
    dat->eventId         = eventId;
    dat->expiration_time = Inter.GetMillisecond() + time;
    CM_NodeLink_Init(&dat->link);
    return dat;
}

/**
 * @brief    删除消息
 * @param    c              协程实例
 * @param    dat            消息数据
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-16
 */
static void DeleteMessage(Coroutine_MailData *dat)
{
    if (dat == nullptr)
        return;
    Inter.Free(dat, __FILE__, __LINE__);
    return;
}

static Coroutine_Mailbox CreateMailbox(const char *name, uint32_t msg_max_size)
{
    CO_Mailbox *mb = (CO_Mailbox *)Inter.Malloc(sizeof(CO_Mailbox), __FILE__, __LINE__);
    if (mb == nullptr) ERROR_MEMORY_ALLOC(__FILE__, __LINE__, sizeof(CO_Mailbox));
    memset(mb, 0, sizeof(CO_Mailbox));
    mb->size  = msg_max_size;
    mb->total = msg_max_size;
    int s     = name == nullptr ? 0 : strlen(name);
    if (s > sizeof(mb->name) - 1) s = sizeof(mb->name) - 1;
    memcpy(mb->name, name, s);
    mb->name[s] = '\0';
    // 加入邮箱列表
    CO_APP_ENTER(&C_Static.cs_mailboxes);
    CM_NodeLink_Insert(&C_Static.mailboxes, CM_NodeLink_End(C_Static.mailboxes), &mb->link);
    CO_APP_LEAVE(&C_Static.cs_mailboxes);
    return mb;
}

static void DeleteMailbox(Coroutine_Mailbox mb)
{
    if (mb == nullptr) return;
    // 删除所有信息
    CO_APP_ENTER(&mb->cs);
    while (!CM_NodeLink_IsEmpty(mb->mails)) {
        Coroutine_MailData *md = CM_Field_ToType(Coroutine_MailData, link, CM_NodeLink_First(mb->mails));
        CM_NodeLink_Remove(&mb->mails, &md->link);
        DeleteMessage(md);
    }
    CO_APP_LEAVE(&mb->cs);
    // 移除邮箱列表
    CO_APP_ENTER(&C_Static.cs_mailboxes);
    CM_NodeLink_Remove(&C_Static.mailboxes, &mb->link);
    CO_APP_LEAVE(&C_Static.cs_mailboxes);
    Inter.Free(mb, __FILE__, __LINE__);
    return;
}

static bool SendMail(Coroutine_Mailbox mb,
                     uint64_t          id,
                     uint64_t          data,
                     uint32_t          size,
                     uint32_t          timeout)
{
    if (mb == nullptr)
        return false;
    CO_APP_ENTER(&mb->cs);
    mb->mail_count++;
    if (mb->size < size + sizeof(Coroutine_MailData)) {
        // 检查邮箱，是否有过期邮件
        uint64_t       now = Inter.GetMillisecond();
        CM_NodeLink_t *p   = CM_NodeLink_First(mb->mails);
        while (p != nullptr && mb->size < size) {
            Coroutine_MailData *md = CM_Field_ToType(Coroutine_MailData, link, p);
            if (md->expiration_time <= now) {
                // 删除过期
                mb->size += md->size;
                mb->mail_count--;
                p = CM_NodeLink_Remove(&mb->mails, &md->link);
                DeleteMessage(md);
                if (p == mb->mails)
                    break;
                continue;
            }
            p = p->next;
            if (p == mb->mails)
                break;
        }
        if (mb->size < size) {
            CO_APP_LEAVE(&mb->cs);
            return false;
        }
    }
    Coroutine_MailData *dat = MakeMessage(id, data, size, timeout);
    if (dat == nullptr) {
        CO_APP_LEAVE(&mb->cs);
        return false;
    }
    mb->size -= size;
    // 检查等待列表
    if (!CM_NodeLink_IsEmpty(mb->waits)) {
        CM_NodeLink_t *p = CM_NodeLink_First(mb->waits);
        while (true) {
            MailWaitNode *n = CM_Field_ToType(MailWaitNode, link, p);
            if (n->id_mask & id) {
                // 溢出等待列表
                CM_NodeLink_Remove(&mb->waits, &n->link);
                mb->wait_count--;
                // 设置返回数据
                n->data = dat;
                // 开始执行
                CO_APP_EnterCriticalSection();
                CO_TCB *task     = (CO_TCB *)n->task;
                task->execv_time = 0;
                task->isWaitMail = 0;
                AddTaskList(task, task->priority);
                CO_APP_LeaveCriticalSection();
                dat = nullptr;
                break;
            }
            p = p->next;
            if (p == mb->waits)
                break;
        }
    }
    if (dat == nullptr) {
        CO_APP_LEAVE(&mb->cs);
        return true;
    }
    // 加入消息列表
    CM_NodeLink_Insert(&mb->mails, CM_NodeLink_End(mb->mails), &dat->link);
    CO_APP_LEAVE(&mb->cs);
    return true;
}

static Coroutine_MailData *GetMail(Coroutine_Mailbox mb,
                                   uint64_t          eventId_Mask)
{
    Coroutine_MailData *ret = nullptr;
    uint64_t            now = Inter.GetMillisecond();
    // 检查邮箱内容
    if (!CM_NodeLink_IsEmpty(mb->mails)) {
        CM_NodeLink_t *p = CM_NodeLink_First(mb->mails);
        while (true) {
            Coroutine_MailData *md = CM_Field_ToType(Coroutine_MailData, link, p);
            if (md->expiration_time <= now) {
                // 清除超时邮件
                Coroutine_MailData *t = md;
                p                     = CM_NodeLink_Remove(&mb->mails, &t->link);
                mb->size += t->size;
                mb->mail_count--;
                DeleteMessage(t);
                if (CM_NodeLink_IsEmpty(mb->mails))
                    break;
                continue;
            }
            if (md->eventId & eventId_Mask) {
                // 获取邮件
                ret = md;
                CM_NodeLink_Remove(&mb->mails, &md->link);
                break;
            }
            p = p->next;
            if (p == CM_NodeLink_First(mb->mails))
                break;   // 检查完成
        }
    }
    if (ret) {
        mb->size += ret->size;
        mb->mail_count--;
    }
    return ret;
}

static Coroutine_MailResult ReceiveMail(Coroutine_Mailbox mb,
                                        uint64_t          eventId_Mask,
                                        uint32_t          timeout)
{
    Coroutine_MailResult ret;
    CM_ZERO(&ret);
    CO_Thread *c = GetCurrentThread(-1);
    if (c == nullptr || c->idx_task == nullptr || mb == nullptr)
        return ret;
    CO_TCB *      task = c->idx_task;
    MailWaitNode  tmp;
    MailWaitNode *n = &tmp;
    CM_ZERO(n);
    CO_APP_ENTER(&mb->cs);
    Coroutine_MailData *dat = GetMail(mb, eventId_Mask);
    if (dat)
        goto END;
    // 加入等待列表
    n->id_mask = eventId_Mask;
    n->task    = task;
    n->data    = nullptr;
    n->mailbox = mb;
    CM_NodeLink_Insert(&mb->waits, CM_NodeLink_End(mb->waits), &n->link);
    mb->wait_count++;
    CO_APP_EnterCriticalSection();
    // 设置等待标志
    task->isWaitMail = 1;
    // 设置超时
    CO_SET_TASK_TIME(task, timeout);
    CO_APP_LeaveCriticalSection();
    CO_APP_LEAVE(&mb->cs);
    // 等待消息
    _Yield();
    // 取出消息
    CO_APP_ENTER(&mb->cs);
    if (n->data) {
        dat     = n->data;
        n->data = nullptr;
        mb->size += dat->size;
        mb->mail_count--;
    }
    CO_APP_EnterCriticalSection();
    if (task->isWaitMail) {
        task->isWaitMail = 0;   // 清除等待标志
        CM_NodeLink_Remove(&mb->waits, &n->link);
        mb->wait_count--;
    }
    CO_APP_LeaveCriticalSection();
END:
    CO_APP_LEAVE(&mb->cs);
    if (dat) {
        ret.data = dat->data;
        ret.size = dat->size;
        ret.id   = dat->eventId;
        ret.isOk = true;
        DeleteMessage(dat);
    }
    return ret;
}
#endif

// --------------------------------------------------------------------------------------
//                              |       系统信息打印        |
// --------------------------------------------------------------------------------------

#if COROUTINE_ENABLE_PRINT_INFO
static size_t co_snprintf(char *buf, size_t size, const char *fmt, ...)
{
    if (buf == nullptr || size == 0)
        return 0;
    va_list args;
    va_start(args, fmt);
    size_t len = vsnprintf(buf, size, fmt, args);
    va_end(args);
    if (len > size)
        len = size;
    return len;
}

static int _PrintInfoTask(char *   buf,
                          int      max_size,
                          CO_TCB * p,
                          int      max_stack,
                          int      max_run,
                          int      count,
                          uint64_t run_time)
{
    if (max_size <= 0 || p == nullptr) return 0;
    CheckStack(p);   // 检查栈
    uint64_t ts  = Inter.GetMillisecond();
    int      idx = 0;
    do {
        char stack[32];
        int  len = co_snprintf(stack,
                              sizeof(stack),
                              "%u/%u/%u",
                              p->stack_len * sizeof(STACK_TYPE),
                              p->stack_max * sizeof(STACK_TYPE),
                              p->stack_alloc * sizeof(STACK_TYPE));
        for (; len < max_stack; len++)
            stack[len] = ' ';
        stack[len] = '\0';
        char time[32];
        len = co_snprintf(time,
                          sizeof(time),
                          "%llu(%d%%)",
                          p->run_time,
                          (int)(p->run_time * 100 / run_time));
        for (; len < max_run; len++)
            time[len] = ' ';
        time[len] = '\0';
        idx += co_snprintf(buf + idx, max_size - idx, "%4d ", count + 1);
        if (idx >= max_size)
            break;
        idx += co_snprintf(buf + idx, max_size - idx, "%p ", p);
        if (idx >= max_size)
            break;
        idx += co_snprintf(buf + idx, max_size - idx, "%p ", p->func);
        if (idx >= max_size)
            break;
        idx += co_snprintf(buf + idx, max_size - idx, "%-6d ", p->coroutine == nullptr ? -1 : p->coroutine->co_id);
        if (idx >= max_size)
            break;
        idx += co_snprintf(buf + idx, max_size - idx, "%u|%u ", p->priority, p->init_priority);
        if (idx >= max_size)
            break;
        const char *sta = "SLR";
        if (p->coroutine != nullptr && p->coroutine->idx_task == p)
            sta = "RUN";
        else if (p->isWaitMail)
            sta = "MAI";
        else if (p->isWaitChannel)
            sta = "CHL";
        else if (p->isWaitSem)
            sta = "SEM";
        else if (p->isWaitMutex)
            sta = "MUT";
        else if (p->isDel)
            sta = "DEL";
        idx += co_snprintf(buf + idx, max_size - idx, " %4s  ", sta);
        if (idx >= max_size)
            break;
        idx += co_snprintf(buf + idx, max_size - idx, "%s ", stack);
        if (idx >= max_size)
            break;
        idx += co_snprintf(buf + idx, max_size - idx, "%s ", time);
        if (idx >= max_size)
            break;
        uint64_t tv = p->execv_time == 0 || p->execv_time <= ts ? 0 : p->execv_time - ts;
        if (tv >= (UINT32_MAX >> 1))
            idx += co_snprintf(buf + idx, max_size - idx, "----       ");
        else
            idx += co_snprintf(buf + idx, max_size - idx, "%-10u ", (uint32_t)(tv));
        if (idx >= max_size)
            break;
        tv = p->watchdog == nullptr ? 0 : p->watchdog->expiration_time - Inter.GetMillisecond();
        if (tv >= (UINT32_MAX >> 1) || p->watchdog == nullptr || p->watchdog->expiration_time == 0)
            idx += co_snprintf(buf + idx, max_size - idx, "----       ");
        else
            idx += co_snprintf(buf + idx, max_size - idx, "%-10u ", (uint32_t)tv);
        idx += co_snprintf(buf + idx, max_size - idx, "%-32s", p->name == nullptr ? "" : p->name);
        if (idx >= max_size)
            break;
        idx += co_snprintf(buf + idx, max_size - idx, "\r\n");
    } while (false);
    return idx;
}

/**
 * @brief    显示协程信息
 * @param    c              协程实例
 * @param    buf            缓存
 * @param    max_size       缓存大小
 * @return   int            实际大小
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-16
 */
static int _PrintInfo(char *buf, int max_size, bool isEx)
{
    if (buf == nullptr || max_size <= 0)
        return 0;
    max_size--;
    int idx = 0;
    // ---------------------------------- 标题 ----------------------------------
    idx += co_snprintf(buf + idx, max_size - idx, " SN  ");
    if (sizeof(size_t) == 4)
        idx += co_snprintf(buf + idx, max_size - idx, " TaskId  ");
    else
        idx += co_snprintf(buf + idx, max_size - idx, "    TaskId      ");
    if (sizeof(size_t) == 4)
        idx += co_snprintf(buf + idx, max_size - idx, " Func    ");
    else
        idx += co_snprintf(buf + idx, max_size - idx, "     Func       ");
    idx += co_snprintf(buf + idx, max_size - idx, "co_id ");
    idx += co_snprintf(buf + idx, max_size - idx, "Pri ");
    idx += co_snprintf(buf + idx, max_size - idx, "Status ");
    // 打印堆栈大小
    int max_stack = 20;
    int max_run   = 13;
    idx += co_snprintf(buf + idx, max_size - idx, "Stack ");
    for (int i = 5; i < max_stack; i++)
        buf[idx++] = ' ';
    // 打印运行时间
    idx += co_snprintf(buf + idx, max_size - idx, "Runtime ");
    for (int i = 7; i < max_run; i++)
        buf[idx++] = ' ';
    // 打印等待时间
    idx += co_snprintf(buf + idx, max_size - idx, "WaitTime   ");
    // 打印看门狗时间
    idx += co_snprintf(buf + idx, max_size - idx, "DogTime   ");
    // 打印名称
    idx += co_snprintf(buf + idx, max_size - idx, " Name");
    idx += co_snprintf(buf + idx, max_size - idx, "\r\n");
    CO_EnterCriticalSection();
    // 统计总的运行时间
    uint64_t run_time = 0;
    uint64_t now      = Inter.GetMillisecond();
    for (size_t i = 0; i < Inter.thread_count; i++)
        run_time += now - C_Static.coroutines[i]->start_time - C_Static.coroutines[i]->sleep_time;
    if (run_time == 0) run_time = 1;
    // 获取打印标志
    bool isPrint = false;
    if (!CM_NodeLink_IsEmpty(C_Static.task_list)) {
        CO_TCB *task = CM_Field_ToType(CO_TCB, task_list_link, CM_NodeLink_First(C_Static.task_list));
        isPrint      = task->isPrint;
    }
    isPrint = !isPrint;
    CO_LeaveCriticalSection();
    int count = 0;
    while (true) {
        CO_EnterCriticalSection();
        if (CM_NodeLink_IsEmpty(C_Static.task_list)) {
            CO_LeaveCriticalSection();
            break;
        }
        CO_TCB *task = CM_Field_ToType(CO_TCB, task_list_link, CM_NodeLink_First(C_Static.task_list));
        if (task->isPrint == isPrint) {   // 检查已打印标志
            CO_LeaveCriticalSection();
            break;   // 打印完成
        }
        // 设置下一个打印节点
        CM_NodeLink_Next(C_Static.task_list);
        idx += _PrintInfoTask(buf + idx, max_size - idx, task, max_stack, max_run, count++, run_time);
        task->isPrint = isPrint;   // 设置已打印标志
        CO_LeaveCriticalSection();
    }
    // 显示线程信息
    CM_NodeLink_Foreach_Positive(CO_Thread, link, C_Static.threads, coroutine)
    {
        uint64_t tv = Inter.GetMillisecond() - coroutine->start_time;
        if (tv == 0)
            tv = 1;
        CO_EnterCriticalSection();
        uint64_t run_time = tv - coroutine->sleep_time;
        CO_LeaveCriticalSection();
        int a = run_time * 1000 / tv;
        idx += co_snprintf(buf + idx,
                           max_size - idx,
                           "ThreadId: %llX(%u) RunTime: %llu(%d.%d%%) ms\r\n",
                           (uint64_t)coroutine->ThreadId,
                           coroutine->co_id,
                           run_time,
                           a / 10,
                           a % 10);
    }
    // ----------------------------- 信号 -----------------------------
    idx += co_snprintf(buf + idx, max_size - idx, " SN  ");
    idx += co_snprintf(buf + idx, max_size - idx, "             Name              ");
    idx += co_snprintf(buf + idx, max_size - idx, "Value   ");
    idx += co_snprintf(buf + idx, max_size - idx, "Wait    ");
    idx += co_snprintf(buf + idx, max_size - idx, "\r\n");
    int sn = 0;
    CO_APP_ENTER(&C_Static.cs_semaphores);
    CM_NodeLink_Foreach_Positive(CO_Semaphore, link, C_Static.semaphores, s)
    {
        idx += co_snprintf(buf + idx, max_size - idx, "%5d ", ++sn);
        idx += co_snprintf(buf + idx, max_size - idx, "%-31s ", s->name);
        idx += co_snprintf(buf + idx, max_size - idx, "%-8u ", s->value);
        idx += co_snprintf(buf + idx, max_size - idx, "%-8u ", s->wait_count);
        idx += co_snprintf(buf + idx, max_size - idx, "\r\n");
    }
    CO_APP_LEAVE(&C_Static.cs_semaphores);
    // ----------------------------- 邮箱 -----------------------------
    idx += co_snprintf(buf + idx, max_size - idx, " SN  ");
    idx += co_snprintf(buf + idx, max_size - idx, "             Name              ");
    idx += co_snprintf(buf + idx, max_size - idx, "used         ");
    idx += co_snprintf(buf + idx, max_size - idx, "total   ");
    idx += co_snprintf(buf + idx, max_size - idx, "Wait    ");
    idx += co_snprintf(buf + idx, max_size - idx, "\r\n");
    sn = 0;
    CO_APP_ENTER(&C_Static.cs_mailboxes);
    CM_NodeLink_Foreach_Positive(CO_Mailbox, link, C_Static.mailboxes, mb)
    {
        idx += co_snprintf(buf + idx, max_size - idx, "%5d ", ++sn);
        idx += co_snprintf(buf + idx, max_size - idx, "%-31s ", mb->name);
        idx += co_snprintf(buf + idx, max_size - idx, "%8u|%-4u ", mb->total - mb->size, mb->mail_count);
        idx += co_snprintf(buf + idx, max_size - idx, "%-8u ", mb->total);
        idx += co_snprintf(buf + idx, max_size - idx, "%-8u ", mb->wait_count);
        idx += co_snprintf(buf + idx, max_size - idx, "\r\n");
    }
    CO_APP_LEAVE(&C_Static.cs_mailboxes);
    // ----------------------------- 互斥 -----------------------------
    idx += co_snprintf(buf + idx, max_size - idx, " SN  ");
    idx += co_snprintf(buf + idx, max_size - idx, "             Name              ");
    if (sizeof(size_t) == 4)
        idx += co_snprintf(buf + idx, max_size - idx, "Owner   ");
    else
        idx += co_snprintf(buf + idx, max_size - idx, "Owner           ");
    idx += co_snprintf(buf + idx, max_size - idx, "Value   ");
    idx += co_snprintf(buf + idx, max_size - idx, "Wait    ");
    idx += co_snprintf(buf + idx, max_size - idx, "MaxWaitTime");
    idx += co_snprintf(buf + idx, max_size - idx, "\r\n");
    sn = 0;
    CO_APP_ENTER(&C_Static.cs_mailboxes);
    CM_NodeLink_Foreach_Positive(CO_Mutex, link, C_Static.mutexes, m)
    {
        idx += co_snprintf(buf + idx, max_size - idx, "%5d ", ++sn);
        idx += co_snprintf(buf + idx, max_size - idx, "%-31s ", m->name);
        idx += co_snprintf(buf + idx, max_size - idx, "%p ", m->owner);
        idx += co_snprintf(buf + idx, max_size - idx, "%-8u ", m->value);
        idx += co_snprintf(buf + idx, max_size - idx, "%-8u ", m->wait_count);
        idx += co_snprintf(buf + idx, max_size - idx, "%u ", m->max_wait_time);
        idx += co_snprintf(buf + idx, max_size - idx, "\r\n");
    }
    CO_APP_LEAVE(&C_Static.cs_mailboxes);
    return idx;
}

static int PrintInfo(char *buf, int max_size)
{
    return _PrintInfo(buf, max_size, true);
}
#endif

// --------------------------------------------------------------------------------------
//                              |       信号量        |
// --------------------------------------------------------------------------------------

#if COROUTINE_ENABLE_SEMAPHORE
/**
 * @brief    创建信号量
 * @param    init_val       初始值
 * @return   Coroutine_Semaphore
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-17
 */
static Coroutine_Semaphore CreateSemaphore(const char *name, uint32_t init_val)
{
    CO_Semaphore *sem = (CO_Semaphore *)Inter.Malloc(sizeof(CO_Semaphore),
                                                     __FILE__,
                                                     __LINE__);
    if (sem == nullptr) ERROR_MEMORY_ALLOC(__FILE__, __LINE__, sizeof(CO_Semaphore));
    memset(sem, 0, sizeof(CO_Semaphore));
    sem->value = init_val;
    sem->list  = nullptr;
    int s      = name == nullptr ? 0 : strlen(name);
    if (s > sizeof(sem->name) - 1) s = sizeof(sem->name) - 1;
    memcpy(sem->name, name, s);
    sem->name[s] = '\0';
    // 加入信号列表
    CO_APP_ENTER(&C_Static.cs_semaphores);
    CM_NodeLink_Insert(&C_Static.semaphores,
                       CM_NodeLink_End(C_Static.semaphores),
                       &sem->link);
    CO_APP_LEAVE(&C_Static.cs_semaphores);
    return sem;
}

/**
 * @brief    删除信号量
 * @param    c              协程实例
 * @param    _sem           信号量
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-17
 */
static void DeleteSemaphore(Coroutine_Semaphore _sem)
{
    CO_Semaphore *sem = (CO_Semaphore *)_sem;
    if (sem == nullptr)
        return;
    CO_APP_ENTER(&sem->cs);
    if (!CM_NodeLink_IsEmpty(sem->list))
        ERROR_SEM_DELETE(sem);
    CO_APP_LEAVE(&sem->cs);
    CO_APP_ENTER(&C_Static.cs_semaphores);
    // 移出列表
    CM_NodeLink_Remove(&C_Static.semaphores, &sem->link);
    CO_APP_LEAVE(&C_Static.cs_semaphores);
    Inter.Free(sem, __FILE__, __LINE__);
    return;
}

/**
 * @brief    给予信号量
 * @param    _sem           信号量
 * @param    val            数值
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-17
 */
static void GiveSemaphore(Coroutine_Semaphore _sem, uint32_t val)
{
    CO_Semaphore *sem = (CO_Semaphore *)_sem;
    if (sem == nullptr || val == 0)
        return;
    CO_APP_ENTER(&sem->cs);
    sem->value += val;
    while (!CM_NodeLink_IsEmpty(sem->list)) {
        SemaphoreNode *n = CM_Field_ToType(SemaphoreNode, link, CM_NodeLink_First(sem->list));
        if (n == nullptr || n->number > sem->value)
            break;
        sem->value -= n->number;
        n->isOk = true;
        // 移除等待列表
        CM_NodeLink_Remove(&sem->list, &n->link);
        sem->wait_count--;
        // 加入运行列表
        CO_APP_EnterCriticalSection();
        CO_TCB *task = (CO_TCB *)n->task;
        // 清除等待标志
        task->isWaitSem  = 0;
        task->execv_time = 0;
        AddTaskList(task, task->priority);
        CO_APP_LeaveCriticalSection();
    }
    CO_APP_LEAVE(&sem->cs);
    return;
}

/**
 * @brief    等待信号量
 * @param    _sem           信号量
 * @param    val            数值
 * @param    timeout        超时
 * @return   true
 * @return   false
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-17
 */
static bool WaitSemaphore(Coroutine_Semaphore _sem, uint32_t val, uint32_t timeout)
{
    if (val == 0)
        return true;
    CO_Semaphore *sem = (CO_Semaphore *)_sem;
    CO_Thread *   c   = GetCurrentThread(-1);
    if (sem == nullptr || c == nullptr || c->idx_task == nullptr)
        return false;
    CO_TCB *      task = c->idx_task;
    bool          isOk = false;
    uint64_t      now  = Inter.GetMillisecond();
    SemaphoreNode tmp;
    do {
        // 计算剩余等待时间
        uint64_t tv = Inter.GetMillisecond() - now;
        if (tv >= (uint64_t)timeout)
            tv = 0;
        else
            tv = timeout - tv;
        // 检查信号值，没有就加入等待列表
        SemaphoreNode *n = &tmp;
        CM_ZERO(n);
        CO_APP_ENTER(&sem->cs);
        if (sem->value >= val) {
            sem->value -= val;
            isOk = true;
        } else {
            n->task      = task;
            n->isOk      = false;
            n->number    = val;
            n->semaphore = sem;
            // 加入等待列表
            CM_NodeLink_Insert(&sem->list, CM_NodeLink_End(sem->list), &n->link);
            // 计数
            sem->wait_count++;
            CO_APP_EnterCriticalSection();
            // 设置等待标志
            task->isWaitSem = true;
            // 设置超时
            CO_SET_TASK_TIME(task, tv);
            CO_APP_LeaveCriticalSection();
        }
        CO_APP_LEAVE(&sem->cs);
        if (isOk) {
            // 已经获取到信号，直接返回
            return true;
        }
        // 等待
        _Yield();
        CO_APP_ENTER(&sem->cs);
        if (n->isOk)
            isOk = true;
        CO_APP_EnterCriticalSection();
        if (task->isWaitSem) {
            task->isWaitSem = false;
            // 移除等待列表
            CM_NodeLink_Remove(&sem->list, &n->link);
            // 设置计数器
            sem->wait_count--;
        }
        CO_APP_LeaveCriticalSection();
        CO_APP_LEAVE(&sem->cs);
    } while (!isOk && Inter.GetMillisecond() - now < timeout);
    return isOk;
}
#endif

// --------------------------------------------------------------------------------------
//                              |       互斥        |
// --------------------------------------------------------------------------------------

#if COROUTINE_ENABLE_MUTEX
static Coroutine_Mutex CreateMutex(const char *name)
{
    CO_Mutex *mutex = (CO_Mutex *)Inter.Malloc(sizeof(CO_Mutex), __FILE__, __LINE__);
    if (mutex == nullptr) ERROR_MEMORY_ALLOC(__FILE__, __LINE__, sizeof(CO_Mutex));
    memset(mutex, 0, sizeof(CO_Mutex));
    int s = name == nullptr ? 0 : strlen(name);
    if (s > sizeof(mutex->name) - 1) s = sizeof(mutex->name) - 1;
    memcpy(mutex->name, name, s);
    mutex->name[s] = '\0';
    // 加入列表
    CO_APP_ENTER(&C_Static.cs_mutexes);
    CM_NodeLink_Insert(&C_Static.mutexes, CM_NodeLink_End(C_Static.mutexes), &mutex->link);
    CO_APP_LEAVE(&C_Static.cs_mutexes);
    return mutex;
}

static void DeleteMutex(Coroutine_Mutex mutex)
{
    if (mutex == nullptr)
        return;
    CO_APP_ENTER(&mutex->cs);
    if (!CM_NodeLink_IsEmpty(mutex->list))
        ERROR_MUTEX_DELETE(mutex);
    CO_APP_LEAVE(&mutex->cs);
    // 移除列表
    CO_APP_ENTER(&C_Static.cs_mutexes);
    CM_NodeLink_Remove(&C_Static.mutexes, &mutex->link);
    CO_APP_LEAVE(&C_Static.cs_mutexes);
    Inter.Free(mutex, __FILE__, __LINE__);
    return;
}

static bool LockMutex(Coroutine_Mutex mutex, uint32_t timeout)
{
    CO_Thread *c = GetCurrentThread(-1);
    if (mutex == nullptr || c == nullptr || c->idx_task == nullptr)
        return false;
    CO_TCB *      task = c->idx_task;
    bool          isOk = false;
    uint64_t      now  = Inter.GetMillisecond();
    MutexWaitNode wait;
    CM_ZERO(&wait);
    wait.task = task;
    do {
        // 计算剩余等待时间
        uint64_t tv = Inter.GetMillisecond() - now;
        if (tv >= (uint64_t)timeout)
            tv = 0;
        else
            tv = timeout - tv;
        CO_APP_ENTER(&mutex->cs);
        MutexWaitNode *wait_mutex = &wait;
        if (mutex->owner == task) {
            // 已经锁定
            mutex->value++;
            isOk = true;
        } else if (mutex->owner == nullptr) {
            // 获取锁
            mutex->owner      = task;
            mutex->value      = 1;
            wait_mutex->mutex = mutex;
            isOk              = true;
        } else {
            CO_APP_EnterCriticalSection();
            CO_TCB *owner = mutex->owner;
            if (owner->priority > task->priority) {
                // 优先级提升
                if (owner->isAddRunList || owner->isAddRunStandaloneList)
                    AddTaskList(owner, task->priority);
                else
                    owner->priority = task->priority;
            }
            CO_APP_LeaveCriticalSection();
            // 等待锁
            CM_NodeLink_Insert(&mutex->list, CM_NodeLink_End(mutex->list), &wait_mutex->link);
            wait_mutex->mutex = mutex;
            wait_mutex->time  = now;
            mutex->wait_count++;
            CO_APP_EnterCriticalSection();
            // 设置等待标志
            task->isWaitMutex = true;
            // 设置超时
            CO_SET_TASK_TIME(task, tv);
            CO_APP_LeaveCriticalSection();
        }
        CO_APP_LEAVE(&mutex->cs);
        if (isOk) return true;   // 获取锁成功
        // 等待
        _Yield();
        CO_APP_ENTER(&mutex->cs);
        // 移除等待列表
        CM_NodeLink_Remove(&mutex->list, &wait_mutex->link);
        CO_APP_EnterCriticalSection();
        if (task->isWaitMutex) {
            mutex->wait_count--;
            task->isWaitMutex = false;
        }
        CO_APP_LeaveCriticalSection();
        isOk = mutex->owner == task;
        CO_APP_LEAVE(&mutex->cs);
    } while (!isOk && (Inter.GetMillisecond() - now) < timeout);
    return isOk;
}

static void UnlockMutex(Coroutine_Mutex mutex)
{
    CO_Thread *c = GetCurrentThread(-1);
    if (mutex == nullptr || c == nullptr || c->idx_task == nullptr)
        return;
    CO_TCB *task = c->idx_task;
    CO_APP_ENTER(&mutex->cs);
    if (mutex->owner == task) {
        // 解锁
        mutex->value--;
        if (mutex->value == 0) {
            // 恢复优先级
            task->priority = task->init_priority;
            // 清除拥有者
            mutex->owner = nullptr;
            // 唤醒等待队列
            if (!CM_NodeLink_IsEmpty(mutex->list)) {
                MutexWaitNode *n = CM_Field_ToType(MutexWaitNode, link, CM_NodeLink_First(mutex->list));
                task             = n->task;
                // 移除等待列表
                CM_NodeLink_Remove(&mutex->list, &n->link);
                // 计数等待时间
                uint64_t tv  = n->time;
                uint64_t now = Inter.GetMillisecond();
                if (now <= tv)
                    tv = 0;
                else
                    tv = now - tv;
                // 设置拥有者
                mutex->owner = task;
                mutex->value = 1;
                mutex->wait_count--;
                CO_APP_EnterCriticalSection();
                // 清除等待标志
                task->execv_time  = 0;
                task->isWaitMutex = false;
                // 加入等待列表
                AddTaskList(task, task->priority);
                CO_APP_LeaveCriticalSection();
                // 获取最大等待时间
                if (mutex->max_wait_time < tv)
                    mutex->max_wait_time = tv;
            }
        }
    } else
        ERROR_MUTEX_RELIEVE(task, mutex);   // 解锁异常
    CO_APP_LEAVE(&mutex->cs);
    return;
}
#endif

static int __tasks_sleep_cm_rbtree_callback_compare(const CM_RBTree_t *     tree,
                                                    const CM_RBTree_Link_t *new_val,
                                                    const CM_RBTree_Link_t *val)
{
    CO_TCB *new_task = CM_Field_ToType(CO_TCB, sleep_link, new_val);
    CO_TCB *task     = CM_Field_ToType(CO_TCB, sleep_link, val);
    if (new_task->execv_time < task->execv_time)
        return TREE_LEFT;
    return TREE_RIGHT;
}

/**
 * @brief    创建协程
 * @return   Coroutine_Handle    nullptr 表示创建失败
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-15
 */
static Coroutine_Handle Coroutine_Create(size_t id)
{
    CO_Thread *c = (CO_Thread *)Inter.Malloc(sizeof(CO_Thread), __FILE__, __LINE__);
    if (c == nullptr) ERROR_MEMORY_ALLOC(__FILE__, __LINE__, sizeof(CO_Thread));
    CM_ZERO(c);
    c->isRun = true;
#if COROUTINE_ENABLE_PRINT_INFO
    c->start_time = Inter.GetMillisecond();
#endif
    c->ThreadId = id;
    CM_RBTree_Init(&c->tasks_sleep, __tasks_sleep_cm_rbtree_callback_compare);
    CO_EnterCriticalSection();
    CM_NodeLink_Insert(&C_Static.threads, CM_NodeLink_End(C_Static.threads), &c->link);
    CO_LeaveCriticalSection();
    return c;
}

static uint64_t GetMillisecond(void)
{
    return Inter.GetMillisecond();
}

static void *Malloc(size_t      size,
                    const char *file,
                    int         line)
{
    void *ptr = Inter.Malloc(size, file, line);
    if (ptr == nullptr) ERROR_MEMORY_ALLOC(file, line, size);
    return ptr;
}

static void Free(void *      ptr,
                 const char *file,
                 int         line)
{
    if (ptr) Inter.Free(ptr, file, line);
    return;
}

static int __watchdogs_cm_rbtree_callback_compare(const CM_RBTree_t *     tree,
                                                  const CM_RBTree_Link_t *new_val,
                                                  const CM_RBTree_Link_t *val)
{
    WatchdogNode *new_task = CM_Field_ToType(WatchdogNode, link, new_val);
    WatchdogNode *task     = CM_Field_ToType(WatchdogNode, link, val);
    if (new_task->expiration_time < task->expiration_time)
        return TREE_LEFT;
    return TREE_RIGHT;
}

static void SetInter(const Coroutine_Inter *inter)
{
    Inter = *inter;
    if (inter->thread_count > 65535) Inter.thread_count = 65535;
    C_Static.coroutines = (CO_Thread **)Inter.Malloc(inter->thread_count * sizeof(CO_Thread *), __FILE__, __LINE__);
    if (C_Static.coroutines == nullptr) ERROR_MEMORY_ALLOC(__FILE__, __LINE__, inter->thread_count * sizeof(CO_Thread *));
    memset(C_Static.coroutines, 0, inter->thread_count * sizeof(CO_Thread *));
    // 创建协程控制器
    for (uint16_t i = 0; i < inter->thread_count; i++) {
        C_Static.coroutines[i]        = Coroutine_Create((size_t)-1);
        C_Static.coroutines[i]->co_id = i;
    }
    // 初始化看门狗列表
    CM_RBTree_Init(&C_Static.watchdogs, __watchdogs_cm_rbtree_callback_compare);
    // 默认堆栈大小
    C_Static.def_stack_size = coroutine_get_stack_default_size();
    return;
}

static const char *GetTaskName(Coroutine_TaskId taskId)
{
    return taskId == nullptr || taskId->name == nullptr ? "" : taskId->name;
}

// --------------------------------------------------------------------------------------
//                              |       异步        |
// --------------------------------------------------------------------------------------

#if COROUTINE_ENABLE_ASYNC
static void _ASyncTask(void *obj)
{
    Coroutine_ASync a = (Coroutine_ASync)obj;
    if (a->func)
        a->ret = a->func(a->object);
    Coroutine.GiveSemaphore(a->sem, 1);
    return;
}

static Coroutine_ASync ASync(Coroutine_AsyncTask func, void *arg, uint32_t stack_size)
{
    if (func == nullptr)
        return nullptr;
    char name[32];
    snprintf(name, sizeof(name), "[%p]", func);
    CO_ASync *a = (CO_ASync *)Inter.Malloc(sizeof(CO_ASync), __FILE__, __LINE__);
    if (a == nullptr) ERROR_MEMORY_ALLOC(__FILE__, __LINE__, sizeof(CO_ASync));
    memset(a, 0, sizeof(CO_ASync));
    a->sem    = Coroutine.CreateSemaphore(name, 0);
    a->func   = func;
    a->object = arg;
    Coroutine_TaskId        taskId;
    Coroutine_TaskAttribute atr;
    memset(&atr, 0, sizeof(Coroutine_TaskAttribute));
    atr.stack_size = stack_size;
    atr.pri        = TASK_PRI_NORMAL;
    atr.co_idx     = -1;
    // 添加任务
    if ((taskId = Coroutine.AddTask(_ASyncTask, a, name, &atr)) == nullptr) {
        Coroutine.DeleteSemaphore(a->sem);
        Inter.Free(a, __FILE__, __LINE__);
        return nullptr;
    }
    snprintf(a->sem->name, sizeof(a->sem->name), "[%p:%llX]", func, (uint64_t)taskId);
    return a;
}

static bool ASyncWait(Coroutine_ASync async, uint32_t timeout)
{
    if (async == nullptr)
        return false;
    if (async->func == nullptr)
        return true;
    if (Coroutine.WaitSemaphore(async->sem, 1, timeout))
        async->func = nullptr;   // 设置执行完成标志
    return async->func == nullptr;
}

static void *ASyncGetResultAndDelete(Coroutine_ASync async_ptr)
{
    if (async_ptr == nullptr)
        return nullptr;
    Coroutine_ASync p = async_ptr;
    if (p->func != nullptr)
        return nullptr;   // 还没执行完
    void *ret = p->ret;
    Coroutine.DeleteSemaphore(p->sem);
    Inter.Free(p, __FILE__, __LINE__);
    return ret;
}
#endif

// --------------------------------------------------------------------------------------
//                              |       看门狗        |
// --------------------------------------------------------------------------------------

#if COROUTINE_ENABLE_WATCHDOG
static void FeedDog(uint32_t time)
{
    CO_Thread *c = GetCurrentThread(-1);
    if (c == nullptr || c->idx_task == nullptr)
        return;
    CO_TCB *task = c->idx_task;
    if (task->watchdog == nullptr) {
        task->watchdog = (WatchdogNode *)Inter.Malloc(sizeof(WatchdogNode), __FILE__, __LINE__);
        if (task->watchdog == nullptr) ERROR_MEMORY_ALLOC(__FILE__, __LINE__, sizeof(WatchdogNode));
        CM_ZERO(task->watchdog);
    }
    // 从已有的列表中删除
    CO_APP_ENTER(&C_Static.cs_watchdogs);
    CM_RBTree_Remove(&C_Static.watchdogs, &task->watchdog->link);
    if (C_Static.idx_watchdog == task) {
        if (!CM_RBTree_IsEmpty(&C_Static.watchdogs)) {
            WatchdogNode *n       = CM_Field_ToType(WatchdogNode, link, CM_RBTree_LeftEnd(&C_Static.watchdogs));
            C_Static.idx_watchdog = n == nullptr ? nullptr : n->task;
        } else
            C_Static.idx_watchdog = nullptr;
    }
    CO_APP_LEAVE(&C_Static.cs_watchdogs);
    CO_EnterCriticalSection();
    // 设置超时时间
    if (time == 0)
        task->watchdog->expiration_time = 0;
    else
        task->watchdog->expiration_time = GetMillisecond() + time;
    CO_LeaveCriticalSection();
    if (task->watchdog->expiration_time) {
        // 添加到新的列表
        CO_APP_ENTER(&C_Static.cs_watchdogs);
        CM_RBTree_Insert(&C_Static.watchdogs, &task->watchdog->link);
        if (C_Static.idx_watchdog == nullptr ||
            task->watchdog->expiration_time < C_Static.idx_watchdog->watchdog->expiration_time)
            C_Static.idx_watchdog = task;
        CO_APP_LEAVE(&C_Static.cs_watchdogs);
    }
    return;
}
#endif

static void SetDefaultStackSize(uint32_t size)
{
    C_Static.def_stack_size = size;
}

// --------------------------------------------------------------------------------------
//                              |       管道        |
// --------------------------------------------------------------------------------------

#if COROUTINE_ENABLE_CHANNEL
static Coroutine_Channel CreateChannel(const char *name, uint32_t size)
{
    CO_Channel *ch = (CO_Channel *)Inter.Malloc(sizeof(CO_Channel), __FILE__, __LINE__);
    if (ch == nullptr) ERROR_MEMORY_ALLOC(__FILE__, __LINE__, sizeof(CO_Channel));
    CM_ZERO(ch);
    int s = name == nullptr ? 0 : strlen(name);
    if (s > sizeof(ch->name) - 1) s = sizeof(ch->name) - 1;
    memcpy(ch->name, name, s + 1);
    ch->size = size;
    // 加入列表
    CO_EnterCriticalSection();
    CM_NodeLink_Insert(&C_Static.channels, CM_NodeLink_End(C_Static.channels), &ch->link);
    CO_LeaveCriticalSection();
    return ch;
}

static void DeleteChannel(Coroutine_Channel ch)
{
    if (ch == nullptr)
        return;
    // 清除缓存
    CO_APP_ENTER(&ch->cs);
    while (!CM_NodeLink_IsEmpty(ch->caches)) {
        ChannelDataNode *dat = CM_Field_ToType(ChannelDataNode, link, CM_NodeLink_First(ch->caches));
        CM_NodeLink_Remove(&ch->caches, &dat->link);
        Inter.Free(dat, __FILE__, __LINE__);
    }
    CO_APP_LEAVE(&ch->cs);
    // 移除列表
    CO_EnterCriticalSection();
    CM_NodeLink_Remove(&C_Static.channels, &ch->link);
    CO_LeaveCriticalSection();
    // 释放内存
    Inter.Free(ch, __FILE__, __LINE__);
    return;
}

static bool WriteChannel(Coroutine_Channel ch, uint64_t data, uint32_t timeout)
{
    if (ch == nullptr)
        return false;
    CO_Thread *c = GetCurrentThread(-1);
    if (c == nullptr || c->idx_task == nullptr)
        return false;
    CO_TCB *        task = c->idx_task;
    bool            isOk = false;
    uint64_t        now  = Inter.GetMillisecond();
    ChannelWaitNode tmp;
    tmp.isOk = false;
    do {
        // 计算剩余等待时间
        uint64_t tv = Inter.GetMillisecond() - now;
        if (tv >= (uint64_t)timeout)
            tv = 0;
        else
            tv = timeout - tv;
        CO_APP_ENTER(&ch->cs);
        if (!CM_NodeLink_IsEmpty(ch->waits)) {
            // 唤醒等待任务
            ChannelWaitNode *n = CM_Field_ToType(ChannelWaitNode, link, CM_NodeLink_First(ch->waits));
            // 移除等待列表
            CM_NodeLink_Remove(&ch->waits, &n->link);
            // 清除标志
        CO_APP_EnterCriticalSection();
            n->task->execv_time    = 0;
            n->task->isWaitChannel = 0;
            n->data                = data;
            n->isOk                = true;
            // 加入运行列表
            AddTaskList(n->task, n->task->priority);
        CO_APP_LeaveCriticalSection();
            // 发送完成
            isOk = true;
        } else if (ch->size > 0) {
            // 加入缓存
            ChannelDataNode *dat = (ChannelDataNode *)Inter.Malloc(sizeof(ChannelDataNode), __FILE__, __LINE__);
            if (dat == nullptr) ERROR_MEMORY_ALLOC(__FILE__, __LINE__, sizeof(ChannelDataNode));
            CM_ZERO(dat);
            dat->data = data;
            CM_NodeLink_Insert(&ch->caches, CM_NodeLink_End(ch->caches), &dat->link);
            // 发送完成
            isOk = true;
            ch->size--;
        } else {
            // 加入发送列表
            ChannelWaitNode *n = &tmp;
            CM_ZERO(n);
            n->task = task;
            n->data = data;
            CM_NodeLink_Insert(&ch->senders, CM_NodeLink_End(ch->senders), &n->link);
        CO_APP_EnterCriticalSection();
            // 设置等待标志
            task->isWaitChannel = 1;
            // 设置任务超时
            CO_SET_TASK_TIME(task, tv);
        CO_APP_LeaveCriticalSection();
        }
        CO_APP_LEAVE(&ch->cs);
        if (isOk) break;
        // 等待
        _Yield();
        CO_APP_ENTER(&ch->cs);
        CO_APP_EnterCriticalSection();
        if (task->isWaitChannel) {
            CM_NodeLink_Remove(&ch->senders, &tmp.link);
            task->isWaitChannel = 0;
        }
        CO_APP_LeaveCriticalSection();
        isOk = tmp.isOk;
        CO_APP_LEAVE(&ch->cs);
    } while (!isOk && (Inter.GetMillisecond() - now) < timeout);
    return isOk;
}

static bool ReadChannel(Coroutine_Channel ch, uint64_t *data, uint32_t timeout)
{
    if (ch == nullptr || data == nullptr)
        return false;
    CO_Thread *c = GetCurrentThread(-1);
    if (c == nullptr || c->idx_task == nullptr)
        return false;
    CO_TCB *        task = c->idx_task;
    bool            isOk = false;
    uint64_t        now  = Inter.GetMillisecond();
    ChannelWaitNode tmp;
    tmp.isOk = false;
    do {
        // 计算剩余等待时间
        uint64_t tv = Inter.GetMillisecond() - now;
        if (tv >= (uint64_t)timeout)
            tv = 0;
        else
            tv = timeout - tv;
        CO_APP_ENTER(&ch->cs);
        if (!CM_NodeLink_IsEmpty(ch->caches)) {
            // 检查缓存
            ChannelDataNode *dat = CM_Field_ToType(ChannelDataNode, link, CM_NodeLink_First(ch->caches));
            *data                = dat->data;
            // 移除缓存
            CM_NodeLink_Remove(&ch->caches, &dat->link);
            // 释放内存
            Inter.Free(dat, __FILE__, __LINE__);
            // 接收完成
            isOk = true;
            ch->size++;
        } else if (!CM_NodeLink_IsEmpty(ch->senders)) {
            // 获取数据
            ChannelWaitNode *n = CM_Field_ToType(ChannelWaitNode, link, CM_NodeLink_First(ch->senders));
            // 移除发送列表
            CM_NodeLink_Remove(&ch->senders, &n->link);
            CO_APP_EnterCriticalSection();
            // 清除标志
            n->task->execv_time    = 0;
            n->task->isWaitChannel = 0;
            *data                  = n->data;
            n->isOk                = true;
            // 加入运行列表
            AddTaskList(n->task, n->task->priority);
            CO_APP_LeaveCriticalSection();
            // 发送完成
            isOk = true;
        } else {
            // 加入等待列表
            ChannelWaitNode *n = &tmp;
            CM_ZERO(n);
            n->task = task;
            CM_NodeLink_Insert(&ch->waits, CM_NodeLink_End(ch->waits), &n->link);
            CO_APP_EnterCriticalSection();
            // 设置等待标志
            task->isWaitChannel = 1;
            // 设置任务超时
            CO_SET_TASK_TIME(task, tv);
            CO_APP_LeaveCriticalSection();
        }
        CO_APP_LEAVE(&ch->cs);
        if (isOk) break;
        // 等待
        _Yield();
        CO_APP_ENTER(&ch->cs);
        CO_APP_EnterCriticalSection();
        if (task->isWaitChannel) {
            // 移除等待列表
            CM_NodeLink_Remove(&ch->waits, &tmp.link);
            task->isWaitChannel = 0;
        }
        if (tmp.isOk) {
            *data = tmp.data;
            isOk  = true;
        }
        CO_APP_LeaveCriticalSection();
        CO_APP_LEAVE(&ch->cs);
    } while (!isOk && (Inter.GetMillisecond() - now) < timeout);
    return isOk;
}
#endif

// --------------------------------------------------------------------------------------
//                              |       通用接口实现        |
// --------------------------------------------------------------------------------------

#if COROUTINE_ENABLE_SEMAPHORE && COROUTINE_ENABLE_MUTEX && COROUTINE_ENABLE_ASYNC && COROUTINE_ENABLE_WATCHDOG
static void Universal_DeleteLock(void *lock)
{
    DeleteMutex((Coroutine_Mutex)lock);
}

static void *Universal_CreateLock(const char *name)
{
    return CreateMutex(name);
}

static bool Universal_Lock(void *obj, uint32_t timeout)
{
    return LockMutex((Coroutine_Mutex)obj, timeout);
}

static void Universal_UnLock(void *mutex)
{
    UnlockMutex((Coroutine_Mutex)mutex);
}

static void *Universal_CreateSemaphore(const char *name, int num)
{
    return CreateSemaphore(name, num);
}

static void Universal_DeleteSemaphore(void *sem)
{
    DeleteSemaphore((Coroutine_Semaphore)sem);
}

static bool Universal_Wait(void *sem, int num, uint32_t time)
{
    return WaitSemaphore((Coroutine_Semaphore)sem, num, time);
}

static void Universal_Give(void *sem, int num)
{
    GiveSemaphore((Coroutine_Semaphore)sem, num);
}

static size_t Universal_RunTask(void (*fun)(void *pars),
                                void *      pars,
                                uint32_t    stacks,
                                uint8_t     pri,
                                const char *name)
{
    Coroutine_TaskAttribute attr;
    memset(&attr, 0, sizeof(Coroutine_TaskAttribute));
    attr.stack_size = stacks;
    attr.pri        = pri;
    attr.co_idx     = -1;
    return (size_t)Coroutine_AddTask(fun, pars, name, &attr);
}

static size_t Universal_GetTaskId(void)
{
    return (size_t)Coroutine_GetTaskId();
}

static void *Universal_Async(void *(*fun)(void *pars), void *pars, uint32_t stacks)
{
    return (void *)ASync(fun, pars, stacks);
}

static bool Universal_AsyncWait(void *async, uint32_t timeout)
{
    return ASyncWait((Coroutine_ASync)async, timeout);
}

static void *Universal_AsyncGetResultAndDelete(void *async)
{
    return ASyncGetResultAndDelete((Coroutine_ASync)async);
}

static void Universal_DelayMs(uint32_t time)
{
    Coroutine_YieldTimeOut(time);
}

static const Universal *GetUniversal(void)
{
    static const Universal universal = {
        GetMillisecond,
        Malloc,
        Free,
        Universal_DelayMs,
        Universal_CreateLock,
        Universal_DeleteLock,
        Universal_Lock,
        Universal_UnLock,
        Universal_CreateSemaphore,
        Universal_DeleteSemaphore,
        Universal_Wait,
        Universal_Give,
        Universal_RunTask,
        Universal_GetTaskId,
        Universal_Async,
        Universal_AsyncWait,
        Universal_AsyncGetResultAndDelete,
        FeedDog,
    };
    return &universal;
}
#endif

const _Coroutine Coroutine = {
    SetInter,
    Coroutine_AddTask,
    Coroutine_GetTaskId,
    GetCurrentCoroutineIdx,
    GetThreadId,
    Coroutine_Yield,
    Coroutine_YieldTimeOut,
    Coroutine_RunTick,
#if COROUTINE_ENABLE_MAILBOX
    CreateMailbox,
    DeleteMailbox,
    SendMail,
    ReceiveMail,
#endif
#if COROUTINE_ENABLE_PRINT_INFO
    PrintInfo,
#endif
#if COROUTINE_ENABLE_SEMAPHORE
    CreateSemaphore,
    DeleteSemaphore,
    GiveSemaphore,
    WaitSemaphore,
#endif
#if COROUTINE_ENABLE_MUTEX
    CreateMutex,
    DeleteMutex,
    LockMutex,
    UnlockMutex,
#endif
    GetMillisecond,
    Malloc,
    Free,
    GetTaskName,
#if COROUTINE_ENABLE_ASYNC
    ASync,
    ASyncWait,
    ASyncGetResultAndDelete,
#endif
#if COROUTINE_ENABLE_SEMAPHORE && COROUTINE_ENABLE_MUTEX && COROUTINE_ENABLE_ASYNC && COROUTINE_ENABLE_WATCHDOG
    GetUniversal,
#endif
#if COROUTINE_ENABLE_WATCHDOG
    FeedDog,
#endif
    SetDefaultStackSize,
#if COROUTINE_ENABLE_CHANNEL
    CreateChannel,
    DeleteChannel,
    WriteChannel,
    ReadChannel,
#endif
};
