
#include "Coroutine.h"
#ifdef WIN32
typedef uint32_t jmp_buf[6];
#else
#include <setjmp.h>
#endif
#include "Coroutine.Platform"

typedef int STACK_TYPE;   // 栈类型

#define STACK_SENTRY 0x5A5A5A5A   // 栈哨兵

#define MAX_PRIORITY_NUM 5   // 最大优先级数

// --------------------------------------------------------------------------------------
//                              |   跳转处理    |
// --------------------------------------------------------------------------------------

#ifdef WIN32
// VC 编译器 longjmp 会析构C++ 对象，使用需要自己实现 setjmp / longjmp

/**
 * @brief    保存跳转环境
 * @param    buf            环境
 * @return   int            跳转返回值
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-19
 */
static int setjmp(jmp_buf buf)
{
    __asm {
        mov eax, dword ptr[buf]
        mov dword ptr[eax + 0], ebx   // 保存 ebx
        mov dword ptr[eax + 4], esi   // 保存 esi
        mov dword ptr[eax + 8], edi   // 保存 edi
        mov ecx, ebp
        add ecx, 8
        mov dword ptr[eax + 12], ecx   // 保存 esp
        mov ecx, dword ptr[ebp]
        mov dword ptr[eax + 16], ecx   // 保存 edp
        mov ecx, dword ptr[ebp + 4]   // 获取 调用 setjmp 下一个地址
        mov dword ptr[eax + 20], ecx   // 保存 setjmp 下一个地址
        mov eax, 0   // 默认返回 0
    }
}

/**
 * @brief    执行跳转
 * @param    buf            环境
 * @param    val            跳转返回值
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-19
 */
static void longjmp(jmp_buf buf, int val)
{
    __asm {
        mov eax, dword ptr[val]
        mov edi, dword ptr[buf]
        mov esi, dword ptr[edi + 4]   // 保存 esi
        mov esp, dword ptr[edi + 12]   // 保存 esp
        mov ebp, dword ptr[edi + 16]   // 保存 edp
        mov ecx, dword ptr[edi + 20]   // 保存 setjmp 下一个地址
        mov ebx, dword ptr[edi + 0]   // 保存 ebx
        mov edi, dword ptr[edi + 8]   // 保存 edi
        jmp ecx
    }
    return;
}

#endif

typedef int (*func_setjmp_t)(jmp_buf buf);
typedef void (*func_longjmp_t)(jmp_buf buf, int val);

// --------------------------------------------------------------------------------------
//                              |       应用        |
// --------------------------------------------------------------------------------------

typedef struct _CO_TCB        CO_TCB;          // 任务控制块/任务节点
typedef struct _CO_Thread     CO_Thread;       // 协程线程
typedef struct _CMessage      CO_Message;      // 消息
typedef struct _CO_Semaphore  CO_Semaphore;    // 信号量
typedef struct _SemaphoreNode SemaphoreNode;   // 信号节点
typedef struct _MailWaitNode  MailWaitNode;    // 信号节点
typedef struct _CO_Mailbox    CO_Mailbox;      // 邮箱
typedef struct _CO_ASync      CO_ASync;        // 异步任务
typedef struct _CO_Mutex      CO_Mutex;        // 互斥锁

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
    CM_NodeLinkList_t list;         // 等待列表 _SemaphoreNode
    uint32_t          value;        // 信号值
    uint32_t          wait_count;   // 等待数
    CM_NodeLink_t     link;         // _CO_Semaphore
};

struct _CO_Mutex
{
    char              name[32];
    CM_NodeLinkList_t list;            // 等待列表 _CO_TCB
    uint32_t          value;           // 互斥值
    uint32_t          wait_count;      // 等待数量
    uint32_t          max_wait_time;   // 最大等待时间
    CM_NodeLink_t     link;            // _CO_Mutex
    CO_TCB *          owner;           // 持有者
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
    uint64_t       run_time;                     // 运行时间
    uint32_t       timeout;                      // 超时时间
    uint32_t       stack_len;                    // 当前栈长度
    uint32_t       stack_max;                    // 最大长度
    uint32_t       stack_alloc;                  // 分配长度
    uint16_t       isSharedStack : 1;            // 是否共享栈
    uint16_t       isDel : 1;                    // 是否删除
    uint16_t       isRun : 1;                    // 正在运行
    uint16_t       isWaitMail : 1;               // 正在等待邮件
    uint16_t       isWaitSem : 1;                // 等待信号量
    uint16_t       isWaitMutex : 1;              // 等待互斥量
    uint16_t       isFirst : 1;                  // 首次
    uint16_t       isAddRunList : 1;             // 添加运行列表
    uint16_t       isAddRunStandaloneList : 1;   // 添加运行列表
    uint16_t       isAddSleepList : 1;           // 添加睡眠列表
    uint16_t       isAddStopList : 1;            // 添加停止列表
    uint16_t       isDynamicThread : 1;          // 动态线程
    uint8_t        priority;                     // 当前优先级
    uint8_t        init_priority;                // 初始化优先级
    Coroutine_Task func;                         // 执行
    char *         name;                         // 名称
    void *         obj;                          // 执行参数
    CO_Thread *    coroutine;                    // 父节点
    STACK_TYPE *   stack;                        // 栈缓存
    STACK_TYPE *   p_stack;                      // 栈
    STACK_TYPE *   s, *p;                        // 临时变量

    SemaphoreNode wait_sem;    // 信号等待节点
    MailWaitNode  wait_mail;   // 邮件等待节点
    struct                     //
    {                          //
        CM_NodeLink_t link;    // _CO_TCB
        CO_Mutex *    mutex;   // 等待互斥锁
    } wait_mutex;              // 等待互斥锁节点
    struct
    {
        CM_NodeLink_t link;              // _CO_TCB
        uint64_t      expiration_time;   // 看门狗过期时间 0：不启用
    } watchdog;

    CM_NodeLink_t link;   // _CO_TCB
};

/**
 * @brief    协程线程
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-15
 */
struct _CO_Thread
{
    jmp_buf              env;               // 环境
    uint64_t             start_time;        // 启动时间
    uint64_t             run_time;          // 运行时间
    uint64_t             task_start_time;   // 任务开始时间
    uint8_t              isRun : 1;         // 运行
    uint8_t              isSleep : 1;       // 正在睡眠
    uint8_t              _run_task_cnt;     // 运行任务次数
    volatile STACK_TYPE *stack;             // 栈
    CO_TCB *             idx_task;          // 当前任务
    size_t               ThreadId;          // 当前协程的线程id
    uint16_t             co_id;             // 协程id
    uint32_t             task_count;        // 任务计数

    CM_NodeLinkList_t tasks_run[MAX_PRIORITY_NUM];   // 运行任务列表 CO_TCB，根据优先级
    CM_NodeLinkList_t tasks_sleep;                   // 睡眠任务列表 CO_TCB
    CM_NodeLinkList_t tasks_stop;                    // 停止列表 CO_TCB

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
    CM_NodeLinkList_t waits;        // 等待列表 _MailWaitNode
    CM_NodeLink_t     link;         // _CO_Mailbox
};

struct _CO_ASync
{
    Coroutine_AsyncTask func;
    void *              object;
    void *              ret;
    Coroutine_Semaphore sem;
};

static Coroutine_Inter Inter;   // 外部接口

static struct
{
    CM_NodeLinkList_t threads;               // 协程控制器列表
    CM_NodeLinkList_t semaphores;            // 信号列表
    CM_NodeLinkList_t mailboxes;             // 邮箱列表
    CM_NodeLinkList_t mutexes;               // 互斥列表 _CO_Mutex
    CO_Thread **      coroutines;            // 协程控制器
    volatile CO_TCB **tasks_map;             // 任务映射表 根据协程id 用于上下文切换
    CM_NodeLinkList_t watchdogs;             // 看门狗列表 CO_TCB 从小到大
    uint64_t          check_watchdog_time;   // 看门狗检查时间
#if !COROUTINE_ONLY_SHARED_STACK
    CM_NodeLinkList_t standalone_tasks_run[MAX_PRIORITY_NUM];   // 独立栈 运行任务列表 CO_TCB，根据优先级
    uint32_t          wait_run_standalone_task_count;           // 等待运行任务数量
#endif
    uint32_t def_stack_size;   // 默认栈大小
} C_Static;

#define CO_EnterCriticalSection()                       \
    do {                                                \
        Inter.EnterCriticalSection(__FILE__, __LINE__); \
    } while (false);

#define CO_LeaveCriticalSection()                       \
    do {                                                \
        Inter.LeaveCriticalSection(__FILE__, __LINE__); \
    } while (false);

static void                   DeleteMessage(Coroutine_MailData *dat);
static Coroutine_Handle       Coroutine_Create(size_t id);
volatile static func_setjmp_t _c_setjmp = setjmp;

// 栈错误
#define ERROR_STACK(t)                                                                \
    if (Inter.events->stackError)                                                     \
        Inter.events->stackError(Inter.events->object, (Coroutine_TaskId)t, t->name); \
    while (true) {                                                                    \
        if (Inter.events->Idle)                                                       \
            Inter.events->Idle(UINT32_MAX, Inter.events->object);                     \
    }

// 检查栈哨兵
#define CHECK_STACK_SENTRY(n)                                                          \
    if (n->stack[0] != STACK_SENTRY || n->stack[n->stack_alloc - 1] != STACK_SENTRY) { \
        ERROR_STACK(n);                                                                \
    }

/**
 * @brief    检查任务栈
 * @param    t              
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-07-04
 */
static void CheckStack(volatile CO_TCB *t)
{
    if (t->isSharedStack)
        return;
    uint32_t *p = (uint32_t *)t->stack;
    CHECK_STACK_SENTRY(t);
    uint32_t idx = 1;
    for (; idx < t->stack_alloc - 1; idx++) {
        if (p[idx] != 0xEEEEEEEE)
            break;
    }
    if (idx == 1) {
        /* 栈错误 */
        ERROR_STACK(t);
    }
    uint32_t s = t->stack_alloc - idx;
    if (s > t->stack_max) t->stack_max = s;
    return;
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
    // 释放消息列表
    if (t->isWaitMail) {
        CM_NodeLink_Remove(&t->wait_mail.mailbox->waits, &t->wait_mail.link);
        t->isWaitMail = 0;
        // 清除消息
        if (t->wait_mail.data != nullptr) {
            DeleteMessage(t->wait_mail.data);
            t->wait_mail.data = nullptr;
        }
    }
    // 释放信号列表
    if (t->isWaitSem) {
        CM_NodeLink_Remove(&t->wait_sem.semaphore->list, &t->wait_sem.link);
        t->isWaitSem = 0;
    }
    // 释放互斥
    if (t->isWaitMutex) {
        CM_NodeLink_Remove(&t->wait_mutex.mutex->list, &t->wait_mutex.link);
        t->isWaitMutex = 0;
    }
    // 移除看门狗
    if (t->watchdog.expiration_time)
        CM_NodeLink_Remove(&C_Static.watchdogs, &t->watchdog.link);
    // 释放名称
    if (t->name) Inter.Free(t->name, __FILE__, __LINE__);
    // 释放堆栈
    Inter.Free(t->stack, __FILE__, __LINE__);
    // 释放实例
    Inter.Free(t, __FILE__, __LINE__);
    return;
}

/**
 * @brief    加入任务列表
 * @param    coroutine      
 * @param    task           
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-06-28
 */
static void AddTaskList(CO_TCB *task, uint8_t new_pri)
{
    CO_Thread *coroutine = task->coroutine;
    uint64_t   now       = Inter.GetMillisecond();
    if (task->isAddRunList) {
        CM_NodeLink_Remove(&coroutine->tasks_run[task->priority], &task->link);
        task->isAddRunList = 0;
    } else if (task->isAddSleepList) {
        CM_NodeLink_Remove(&coroutine->tasks_sleep, &task->link);
        task->isAddSleepList = 0;
    } else if (task->isAddStopList) {
        CM_NodeLink_Remove(&coroutine->tasks_stop, &task->link);
        task->isAddStopList = 0;
    } else if (task->isAddRunStandaloneList) {
        CM_NodeLink_Remove(&C_Static.standalone_tasks_run[task->priority], &task->link);
        task->isAddRunStandaloneList = 0;
        C_Static.wait_run_standalone_task_count--;
    }
    if (coroutine != nullptr) coroutine->task_count--;
    if (new_pri != task->priority)
        task->priority = new_pri;   // 更新优先级
    if (task->isRun == 0) {         // 加入停止列表
        coroutine->task_count++;
        CM_NodeLink_Insert(&coroutine->tasks_stop,
                           CM_NodeLink_End(coroutine->tasks_stop),
                           &task->link);
        task->isAddStopList = 1;
    } else if (task->isDel || now >= task->execv_time) {   // 加入运行列表
#if !COROUTINE_ONLY_SHARED_STACK
        if (task->isSharedStack ||
            task->isDynamicThread == 0 ||
            Inter.thread_count == 1 ||
            !coroutine_is_dynamic_run_thread()) {
            // 共享栈，加入本协程控制器
            CM_NodeLink_Insert(&coroutine->tasks_run[task->priority],
                               CM_NodeLink_End(coroutine->tasks_run[task->priority]),
                               &task->link);
            task->isAddRunList = 1;
            coroutine->task_count++;
        } else {
            // 独立栈，加入公共列表
            task->coroutine = nullptr;
            CM_NodeLink_Insert(&C_Static.standalone_tasks_run[task->priority],
                               CM_NodeLink_End(C_Static.standalone_tasks_run[task->priority]),
                               &task->link);
            task->isAddRunStandaloneList = 1;
            C_Static.wait_run_standalone_task_count++;
        }
#else
        CM_NodeLink_Insert(&coroutine->tasks_run[task->priority],
                           CM_NodeLink_End(coroutine->tasks_run[task->priority]),
                           &task->link);
        task->isAddRunList = 1;
        coroutine->wait_run_task_count++;
#endif
    } else {   // 加入休眠列表，从小到大
        coroutine->task_count++;
        if (CM_NodeLink_IsEmpty(coroutine->tasks_sleep)) {
            CM_NodeLink_Init(&task->link);
            coroutine->tasks_sleep = &task->link;
        } else {
            CO_TCB *e = CM_NodeLink_ToType(CO_TCB, link, CM_NodeLink_End(coroutine->tasks_sleep));
            CO_TCB *s = CM_NodeLink_ToType(CO_TCB, link, CM_NodeLink_First(coroutine->tasks_sleep));
            if (e->execv_time <= task->execv_time) {
                // 加入到最后
                CM_NodeLink_Insert(&coroutine->tasks_sleep,
                                   CM_NodeLink_End(coroutine->tasks_sleep),
                                   &task->link);
            } else if (s->execv_time > task->execv_time) {
                // 加入到头部
                CM_NodeLink_Insert(&coroutine->tasks_sleep,
                                   CM_NodeLink_End(coroutine->tasks_sleep),
                                   &task->link);
                coroutine->tasks_sleep = &task->link;
            } else {
                // 遍历寻找插入位置
                CO_TCB *r = e;
                CM_NodeLink_Foreach_Positive(CO_TCB, link, coroutine->tasks_sleep, p)
                {
                    if (p->execv_time > task->execv_time) {
                        r = p;
                        break;
                    }
                }
                CM_NodeLink_Insert(&coroutine->tasks_sleep,
                                   r->link.up,
                                   &task->link);
            }
        }
        task->isAddSleepList = 1;
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
    if (CM_NodeLink_IsEmpty(coroutine->tasks_sleep))
        return 0;
    CO_TCB *task = CM_NodeLink_ToType(CO_TCB, link, CM_NodeLink_First(coroutine->tasks_sleep));
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
        task = CM_NodeLink_ToType(CO_TCB, link, CM_NodeLink_First(coroutine->tasks_run[pri]));
        // 移除运行列表
        CM_NodeLink_Remove(&coroutine->tasks_run[pri], &task->link);
        task->isAddRunList = 0;
        coroutine->task_count--;
    }
    return task;
}

static CO_TCB *GetNextTask_StandaloneTask(CO_Thread *coroutine, uint8_t pri)
{
    CO_TCB *task = nullptr;
    if (!CM_NodeLink_IsEmpty(C_Static.standalone_tasks_run[pri])) {
        task = CM_NodeLink_ToType(CO_TCB, link, CM_NodeLink_First(C_Static.standalone_tasks_run[pri]));
        // 移除运行列表
        CM_NodeLink_Remove(&C_Static.standalone_tasks_run[pri], &task->link);
        task->isAddRunStandaloneList = 0;
        task->coroutine              = coroutine;
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
    coroutine->idx_task = task;
    coroutine->_run_task_cnt++;
    coroutine->task_count++;
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
    if (Inter.GetMillisecond() < C_Static.check_watchdog_time || CM_NodeLink_IsEmpty(C_Static.watchdogs))
        return;
    C_Static.check_watchdog_time = Inter.GetMillisecond() + 10;   // 每 10ms 检查一次
    CO_TCB *t                    = CM_NodeLink_ToType(CO_TCB, watchdog.link, CM_NodeLink_First(C_Static.watchdogs));
    if (t->watchdog.expiration_time < Inter.GetMillisecond()) {
        // 看门狗超时
        if (Inter.events->watchdogTimeout != nullptr)
            Inter.events->watchdogTimeout(Inter.events->object, t, t->name);
    }
    return;
}

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
    for (size_t i = 0; i < Inter.thread_count; i++) {
        if (C_Static.coroutines[idx]->isSleep)
            break;
        idx = (idx + 1) % Inter.thread_count;
    }
    return C_Static.coroutines[idx];
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
            if (n->isSharedStack) {
                // 直接进入函数
                __task((CO_TCB *)n);
            } else {
#if !COROUTINE_ONLY_SHARED_STACK
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
#endif
            }
            // 不会执行到这里
        } else {
            // 回到 SaveStack.setjmp
            C_Static.tasks_map[n->coroutine->co_id] = (CO_TCB *)n;
            longjmp(((CO_TCB *)n)->env, n->coroutine->co_id + 1);
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
    volatile uint32_t flag1    = 0x55667788;
    volatile CO_TCB * n        = nullptr;
    volatile uint32_t sleep_ms = UINT32_MAX;
    volatile uint32_t flag2    = 0x55667788;
    coroutine->task_start_time = Inter.GetMillisecond();
    // 获取下一个任务
    CO_EnterCriticalSection();
    CheckWatchdog();
    sleep_ms = GetSleepTask(coroutine, coroutine->task_start_time);
    GetNextTask(coroutine);
    n = coroutine->idx_task;
    CO_LeaveCriticalSection();
    if (n == nullptr) {
        // 运行空闲任务
        if (sleep_ms > 1 && Inter.events->Idle != nullptr) {
            CO_EnterCriticalSection();
            coroutine->isSleep = 1;   // 设置休眠状态
            CO_LeaveCriticalSection();
            Inter.events->Idle(sleep_ms - 1, Inter.events->object);
            CO_EnterCriticalSection();
            coroutine->isSleep = 0;   // 清除休眠状态
            CO_LeaveCriticalSection();
        }
        return;
    }
    _enter_into(n);
    // 记录运行时间
    uint64_t ts = Inter.GetMillisecond();
    uint64_t tv = ts <= coroutine->task_start_time ? 0 : ts - coroutine->task_start_time;
    coroutine->run_time += tv;
    n->run_time += tv;
    coroutine->idx_task = nullptr;
    // 添加到运行列表
    CO_EnterCriticalSection();
    if (!(n->isAddRunList | n->isAddSleepList | n->isAddStopList | n->isAddRunStandaloneList))
        AddTaskList((CO_TCB *)n, n->priority);
    CO_LeaveCriticalSection();
    // 周期事件
    if (Inter.events->Period != nullptr)
        Inter.events->Period(Inter.events->object);
    (void)flag1;
    (void)flag2;
    return;
}

/**
 * @brief    上下文切换【共享栈】
 * @param    coroutine
 * @param    timeout
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-31
 */
static void ContextSwitch_Shared(volatile CO_TCB *n)
{
    // 保存环境,回到调度器
    int ret = _c_setjmp(((CO_TCB *)n)->env);
    if (ret == 0) {
        volatile STACK_TYPE __mem = 0x11223344;   // 利用局部变量获取堆栈寄存器值
        // 保存栈数据
        n->p_stack = ((STACK_TYPE *)&__mem);   // 获取栈结尾
        n->p_stack -= 4;
        n->stack_len = n->coroutine->stack - n->p_stack;
        if (n->stack_len > n->stack_alloc) {
            if (n->stack != nullptr)
                Inter.Free(n->stack, __FILE__, __LINE__);
            n->stack_alloc = ALIGN(n->stack_len, 128) * 128;   // 按512字节分配，避免内存碎片
            n->stack       = (STACK_TYPE *)Inter.Malloc(n->stack_alloc * sizeof(STACK_TYPE), __FILE__, __LINE__);
        }
        if (n->stack_len > n->stack_max) n->stack_max = n->stack_len;
        if (n->stack == nullptr) {
            // 内存分配错误
            n->isDel = true;
            if (Inter.events->Allocation != nullptr)
                Inter.events->Allocation(__LINE__,
                                         n->stack_len,
                                         Inter.events->object);
            return;
        }
        memcpy((char *)n->stack, (char *)n->p_stack, n->stack_len * sizeof(int));
        // 回到调度器
        longjmp(n->coroutine->env, 1);
    } else {
        n = *(C_Static.tasks_map + ret - 1);
        // 恢复堆栈内容
        n->p = n->p_stack + n->stack_len - 1;
        n->s = n->stack + n->stack_len - 1;
        while (n->s >= n->stack)   // 限制范围
            *(n->p)-- = *(n->s)--;
    }
    return;
}

#if !COROUTINE_ONLY_SHARED_STACK
/**
 * @brief    上下文切换【独立栈】
 * @param    n              
 * @return   * void         
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-07-03
 */
static void ContextSwitch_Standalone(volatile CO_TCB *n)
{
    // 保存环境,回到调度器
    int ret = _c_setjmp(((CO_TCB *)n)->env);
    if (ret == 0) {
        volatile STACK_TYPE __mem = 0x11223344;               // 利用局部变量获取堆栈寄存器值
        n->p_stack                = ((STACK_TYPE *)&__mem);   // 获取栈结尾
        n->p_stack -= 4;
        // 检查栈溢出
        if (n->p_stack <= n->stack) {
            // 栈溢出
            ERROR_STACK(n);
        }
        // 计算栈大小
        n->stack_len = n->stack_alloc - (n->p_stack - n->stack);
        // 记录最大栈大小
        if (n->stack_len > n->stack_max) n->stack_max = n->stack_len;
#if COROUTINE_CHECK_STACK
        CheckStack(n);
#else
        // 检查栈哨兵
        CHECK_STACK_SENTRY(n);
#endif
        // 回到调度器
        longjmp(n->coroutine->env, 1);
    }
    return;
}
#endif

/**
 * @brief    转交控制权
 * @param    coroutine      线程实例
 * @param    timeout        超时
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-15
 */
static void _Yield(CO_Thread *coroutine, uint32_t timeout)
{
    if (coroutine == nullptr)
        return;
    CO_TCB *n = coroutine->idx_task;
    if (n == nullptr)
        return;
    if (n->isRun) {
        if (timeout > 0)
            n->execv_time = Inter.GetMillisecond() + timeout;
        else
            n->execv_time = 0;
        n->timeout = timeout;
    }
    // 保存堆栈数据
    if (n->isSharedStack)
        ContextSwitch_Shared(n);   // 共享栈
#if !COROUTINE_ONLY_SHARED_STACK
    else
        ContextSwitch_Standalone(n);   // 独立栈
#endif
    return;
}

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
    size_t     idx = 0;
    CO_Thread *ret = nullptr;
    if (Inter.thread_count == 1) {
        ret = C_Static.coroutines[0];
        if (ret->ThreadId == (size_t)-1)
            ret->ThreadId = Inter.GetThreadId();
    } else {
        if (co_idx < 0) {
            size_t id  = Inter.GetThreadId();
            size_t _id = id % 65537;
            CO_EnterCriticalSection();
            for (size_t i = 0; i < Inter.thread_count; i++) {
                idx = (_id + i) % Inter.thread_count;
                if (C_Static.coroutines[idx]->ThreadId == id) {
                    ret = C_Static.coroutines[idx];
                    break;
                } else if (C_Static.coroutines[idx]->ThreadId == (size_t)-1) {
                    // 分配控制器
                    ret           = C_Static.coroutines[idx];
                    ret->ThreadId = id;
                    break;
                }
            }
            CO_LeaveCriticalSection();
        } else {
            CO_EnterCriticalSection();
            CM_NodeLink_Foreach_Positive(CO_Thread, link, C_Static.threads, p)
            {
                if (p->co_id == co_idx) {
                    ret = p;
                    break;
                }
            }
            CO_LeaveCriticalSection();
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
                                uint32_t       stack_size,
                                bool           isSharedStack)
{
    if (func == nullptr)
        return nullptr;
    stack_size = ALIGN(stack_size, sizeof(STACK_TYPE));
    CO_TCB *n  = (CO_TCB *)Inter.Malloc(sizeof(CO_TCB), __FILE__, __LINE__);
    if (n == nullptr) {
        if (Inter.events->Allocation != nullptr)
            Inter.events->Allocation(__LINE__, sizeof(CO_TCB), Inter.events->object);
        return nullptr;
    }
    memset(n, 0, sizeof(CO_TCB));
#if COROUTINE_ONLY_SHARED_STACK
    n->isSharedStack = true;
#else
    n->isSharedStack = isSharedStack;
#endif
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
    CM_NodeLink_Init(&n->link);
    if (name != nullptr && name[0] != '\0') {
        n->name = (char *)Inter.Malloc(32, __FILE__, __LINE__);
        if (n->name == nullptr) {
            if (Inter.events->Allocation != nullptr)
                Inter.events->Allocation(__LINE__, sizeof(CO_TCB), Inter.events->object);
            return nullptr;
        }
        int s = strlen(name);
        if (s > 31) s = 31;
        memcpy(n->name, name, s + 1);
    }
#if !COROUTINE_SHARED_STACK
    if (n->isSharedStack == 0) {
        // 初始化栈空间
        memset(n->stack, 0xEE, n->stack_alloc * sizeof(STACK_TYPE));
        n->stack[0] = n->stack[n->stack_alloc - 1] = STACK_SENTRY;   // 设置哨兵
    }
#endif
    CO_EnterCriticalSection();
    // 添加到任务列表
    AddTaskList(n, n->priority);
    uint16_t co_id = coroutine ? coroutine->co_id : GetSleepThread()->co_id;
    CO_LeaveCriticalSection();
    // 唤醒
    if (Inter.events->wake != nullptr)
        Inter.events->wake(co_id, Inter.events->object);
    return n;
}

static Coroutine_TaskId Coroutine_AddTask(Coroutine_Task                 func,
                                          void *                         pars,
                                          const char *                   name,
                                          const Coroutine_TaskAttribute *attr)
{
    CO_Thread *coroutine = nullptr;
    int        co_idx    = attr == nullptr ? -1 : attr->co_idx;
    uint8_t    pri       = attr == nullptr ? TASK_PRI_NORMAL : attr->pri;
#if COROUTINE_ONLY_SHARED_STACK
    bool isSharedStack = true;
#else
    bool isSharedStack = attr == nullptr ? false : attr->isSharedStack;
#endif
    uint32_t stack_size = 0;
    if (isSharedStack) {
        stack_size = attr == nullptr ? 0 / sizeof(STACK_TYPE) : attr->stack_size;   // 默认 512 字节栈空间
        if (stack_size == 0) stack_size = 512;
    } else {
        stack_size = attr == nullptr ? 0 / sizeof(STACK_TYPE) : attr->stack_size;   // 默认 4KB 栈空间
        if (stack_size == 0)
            stack_size = C_Static.def_stack_size;
    }
    if (Inter.thread_count == 1) {
        coroutine = C_Static.coroutines[0];
    } else if (isSharedStack || co_idx >= 0) {
        if (co_idx < 0) {
            // 寻址负载小的线程分配
            uint32_t run_load = UINT32_MAX;   // 运行负载
            uint32_t run_num  = UINT32_MAX;   // 运行数量
            uint64_t now      = Inter.GetMillisecond();
            int      n        = rand() % Inter.thread_count;
            CO_EnterCriticalSection();
            // 随机起始
            CM_NodeLinkList_t list = C_Static.threads;
            for (int i = 0; i < n; i++)
                CM_NodeLink_Next(list);
            CM_NodeLink_Foreach_Positive(CO_Thread, link, list, p)
            {
                uint64_t tv = now - p->start_time;
                if (tv == 0) tv = 1;
                uint32_t load = p->run_time * 1000 / tv;
                if (load < run_load || (load == run_load && p->task_count < run_num)) {
                    run_load  = load;
                    run_num   = p->task_count;
                    coroutine = p;
                }
            }
            CO_LeaveCriticalSection();
        } else
            coroutine = GetCurrentThread(co_idx);
    }
    return AddTask(coroutine, func, pars, pri, name, stack_size, isSharedStack);
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
    _Yield((CO_Thread *)GetCurrentThread(-1), 0);
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
        Coroutine_Yield();
        return 0;
    }
    uint64_t ts = Inter.GetMillisecond();
    _Yield((CO_Thread *)GetCurrentThread(-1), timeout);
    ts = Inter.GetMillisecond() - ts;                   // 计算耗时
    return ts > timeout ? (uint32_t)ts - timeout : 0;   // 返回误差
}

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
static Coroutine_MailData *MakeMessage(uint64_t    eventId,
                                       const void *data,
                                       size_t      size,
                                       uint32_t    time)
{
    size_t mlen             = size + sizeof(Coroutine_MailData);
    mlen                    = ALIGN(mlen, 64) << 6;   // 64B对齐
    Coroutine_MailData *dat = (Coroutine_MailData *)Inter.Malloc(mlen, __FILE__, __LINE__);
    if (dat == nullptr)
        return nullptr;
    dat->data = (char *)(dat + 1);
    if (data != nullptr)
        memcpy(dat->data, data, size);
    else
        memset(dat->data, 0, size);
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
    if (mb == nullptr) return nullptr;
    memset(mb, 0, sizeof(CO_Mailbox));
    mb->size  = msg_max_size;
    mb->total = msg_max_size;
    int s     = name == nullptr ? 0 : strlen(name);
    if (s > sizeof(mb->name) - 1) s = sizeof(mb->name) - 1;
    memcpy(mb->name, name, s);
    mb->name[s] = '\0';
    // 加入邮箱列表
    CO_EnterCriticalSection();
    CM_NodeLink_Insert(&C_Static.mailboxes, CM_NodeLink_End(C_Static.mailboxes), &mb->link);
    CO_LeaveCriticalSection();
    return mb;
}

static void DeleteMailbox(Coroutine_Mailbox mb)
{
    if (mb == nullptr) return;
    CO_EnterCriticalSection();
    // 删除所有信息
    while (!CM_NodeLink_IsEmpty(mb->mails)) {
        Coroutine_MailData *md = CM_NodeLink_ToType(Coroutine_MailData, link, CM_NodeLink_First(mb->mails));
        CM_NodeLink_Remove(&mb->mails, &md->link);
        DeleteMessage(md);
    }
    // 移除邮箱列表
    CM_NodeLink_Remove(&C_Static.mailboxes, &mb->link);
    CO_LeaveCriticalSection();
    Inter.Free(mb, __FILE__, __LINE__);
    return;
}

static bool SendMail(Coroutine_Mailbox mb, Coroutine_MailData *data)
{
    if (mb == nullptr || data == nullptr)
        return false;
    CO_EnterCriticalSection();
    mb->mail_count++;
    if (mb->size < data->size + sizeof(Coroutine_MailData)) {
        // 检查邮箱，是否有过期邮件
        uint64_t       now = Inter.GetMillisecond();
        CM_NodeLink_t *p   = CM_NodeLink_First(mb->mails);
        while (p != nullptr && mb->size < data->size + sizeof(Coroutine_MailData)) {
            Coroutine_MailData *md = CM_NodeLink_ToType(Coroutine_MailData, link, p);
            if (md->expiration_time <= now) {
                // 删除过期
                mb->size += md->size + sizeof(Coroutine_MailData);
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
        if (mb->size < data->size + sizeof(Coroutine_MailData)) {
            CO_LeaveCriticalSection();
            return false;
        }
    }
    mb->size -= data->size + sizeof(Coroutine_MailData);
    uint16_t co_id = 0;
    // 检查等待列表
    if (!CM_NodeLink_IsEmpty(mb->waits)) {
        CM_NodeLink_t *p = CM_NodeLink_First(mb->waits);
        while (true) {
            MailWaitNode *n = CM_NodeLink_ToType(MailWaitNode, link, p);
            if (n->id_mask & data->eventId) {
                n->data = data;
                // 开始执行
                CO_TCB *task     = (CO_TCB *)n->task;
                task->execv_time = 0;
                task->timeout    = 0;
                AddTaskList(task, task->priority);
                data  = nullptr;
                co_id = task->isSharedStack ? task->coroutine->co_id : GetSleepThread()->co_id;
                break;
            }
            p = p->next;
            if (p == mb->waits)
                break;
        }
    }
    if (data == nullptr) {
        CO_LeaveCriticalSection();
        // 唤醒
        if (Inter.events->wake != nullptr)
            Inter.events->wake(co_id, Inter.events->object);
        return true;
    }
    // 加入消息列表
    CM_NodeLink_Insert(&mb->mails, CM_NodeLink_End(mb->mails), &data->link);
    CO_LeaveCriticalSection();
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
            Coroutine_MailData *md = CM_NodeLink_ToType(Coroutine_MailData, link, p);
            if (md->expiration_time <= now) {
                // 超时
                Coroutine_MailData *t = md;
                p                     = CM_NodeLink_Remove(&mb->mails, &t->link);
                mb->size += t->size + sizeof(Coroutine_MailData);
                mb->mail_count--;
                DeleteMessage(t);
                if (p == CM_NodeLink_First(mb->mails))
                    break;
                continue;
            }
            if (md->eventId & eventId_Mask) {
                ret = md;
                CM_NodeLink_Remove(&mb->mails, &md->link);
                break;
            }
            p = p->next;
            if (p == CM_NodeLink_First(mb->mails))
                break;
        }
    }
    if (ret) {
        mb->size += ret->size + sizeof(Coroutine_MailData);
        mb->mail_count--;
    }
    return ret;
}

static Coroutine_MailData *ReceiveMail(Coroutine_Mailbox mb,
                                       uint64_t          eventId_Mask,
                                       uint32_t          timeout)
{
    CO_Thread *c = GetCurrentThread(-1);
    if (c == nullptr || c->idx_task == nullptr || mb == nullptr)
        return nullptr;
    CO_TCB *task = c->idx_task;
    CO_EnterCriticalSection();
    Coroutine_MailData *ret = GetMail(mb, eventId_Mask);
    if (ret) {
        CO_LeaveCriticalSection();
        _Yield(c, 0);
        return ret;
    }
    // 加入等待列表
    task->isWaitMail = 1;   // 设置等待标志
    MailWaitNode *n  = &task->wait_mail;
    n->id_mask       = eventId_Mask;
    n->task          = task;
    n->data          = nullptr;
    n->mailbox       = mb;
    CM_NodeLink_Insert(&mb->waits, CM_NodeLink_End(mb->waits), &n->link);
    mb->wait_count++;
    CO_LeaveCriticalSection();
    // 等待消息
    _Yield(c, timeout);
    // 取出消息
    CO_EnterCriticalSection();
    if (n->data) {
        ret     = n->data;
        n->data = nullptr;
        mb->size += ret->size + sizeof(Coroutine_MailData);
        mb->mail_count--;
    }
    CM_NodeLink_Remove(&mb->waits, &n->link);
    task->isWaitMail = 0;   // 清除等待标志
    mb->wait_count--;
    CO_LeaveCriticalSection();
    return ret;
}

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
        idx += co_snprintf(buf + idx, max_size - idx, "%u|%u ", p->priority, p->init_priority);
        if (idx >= max_size)
            break;
        idx += co_snprintf(buf + idx,
                           max_size - idx,
                           "  %c%c   ",
                           p->isSharedStack ? 'S' : 'M',
                           p->coroutine != nullptr && p->coroutine->idx_task == p ? 'R' : (p->isWaitMail || p->isWaitSem || p->isWaitMutex) ? 'W'
                                                                                                                                            : 'S');
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
        tv = p->watchdog.expiration_time - Inter.GetMillisecond();
        if (tv >= (UINT32_MAX >> 1) || p->watchdog.expiration_time == 0)
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
    uint64_t run_time = 0;
    for (size_t i = 0; i < Inter.thread_count; i++)
        run_time += C_Static.coroutines[i]->run_time;
    if (run_time == 0) run_time = 1;
    CM_NodeLink_Foreach_Positive(CO_Thread, link, C_Static.threads, coroutine)
    {
        int count = 0;
        // ---------------------------------- 内容 ----------------------------------
        if (coroutine->idx_task)
            idx += _PrintInfoTask(buf + idx, max_size - idx, coroutine->idx_task, max_stack, max_run, count++, run_time);
        for (int i = 0; i < TAB_SIZE(coroutine->tasks_run); i++) {
            CM_NodeLink_Foreach_Positive(CO_TCB, link, coroutine->tasks_run[i], task)
            {
                idx += _PrintInfoTask(buf + idx, max_size - idx, task, max_stack, max_run, count++, run_time);
            }
        }
        CM_NodeLink_Foreach_Positive(CO_TCB, link, coroutine->tasks_sleep, task)
        {
            idx += _PrintInfoTask(buf + idx, max_size - idx, task, max_stack, max_run, count++, run_time);
        }
        CM_NodeLink_Foreach_Positive(CO_TCB, link, coroutine->tasks_stop, task)
        {
            idx += _PrintInfoTask(buf + idx, max_size - idx, task, max_stack, max_run, count++, run_time);
        }
        uint64_t tv = Inter.GetMillisecond() - coroutine->start_time;
        if (tv == 0)
            tv = 1;
        int a = coroutine->run_time * 1000 / tv;
        idx += co_snprintf(buf + idx,
                           max_size - idx,
                           "ThreadId: %llX(%u) RunTime: %llu(%d.%d%%) ms Task count: %u\r\n",
                           (uint64_t)coroutine->ThreadId,
                           coroutine->co_id,
                           coroutine->run_time,
                           a / 10,
                           a % 10,
                           count);
    }
#if !COROUTINE_ONLY_SHARED_STACK
    {
        int count = 0;
        for (int i = 0; i < MAX_PRIORITY_NUM; i++) {
            CM_NodeLink_Foreach_Positive(CO_TCB, link, C_Static.standalone_tasks_run[i], task)
            {
                idx += _PrintInfoTask(buf + idx, max_size - idx, task, max_stack, max_run, count++, run_time);
            }
        }
    }
#endif
    // ----------------------------- 信号 -----------------------------
    idx += co_snprintf(buf + idx, max_size - idx, " SN  ");
    idx += co_snprintf(buf + idx, max_size - idx, "             Name              ");
    idx += co_snprintf(buf + idx, max_size - idx, "Value   ");
    idx += co_snprintf(buf + idx, max_size - idx, "Wait    ");
    idx += co_snprintf(buf + idx, max_size - idx, "\r\n");
    int sn = 0;
    CM_NodeLink_Foreach_Positive(CO_Semaphore, link, C_Static.semaphores, s)
    {
        idx += co_snprintf(buf + idx, max_size - idx, "%5d ", ++sn);
        idx += co_snprintf(buf + idx, max_size - idx, "%-31s ", s->name);
        idx += co_snprintf(buf + idx, max_size - idx, "%-8u ", s->value);
        idx += co_snprintf(buf + idx, max_size - idx, "%-8u ", s->wait_count);
        idx += co_snprintf(buf + idx, max_size - idx, "\r\n");
    }
    // ----------------------------- 邮箱 -----------------------------
    idx += co_snprintf(buf + idx, max_size - idx, " SN  ");
    idx += co_snprintf(buf + idx, max_size - idx, "             Name              ");
    idx += co_snprintf(buf + idx, max_size - idx, "used         ");
    idx += co_snprintf(buf + idx, max_size - idx, "total   ");
    idx += co_snprintf(buf + idx, max_size - idx, "Wait    ");
    idx += co_snprintf(buf + idx, max_size - idx, "\r\n");
    sn = 0;
    CM_NodeLink_Foreach_Positive(CO_Mailbox, link, C_Static.mailboxes, mb)
    {
        idx += co_snprintf(buf + idx, max_size - idx, "%5d ", ++sn);
        idx += co_snprintf(buf + idx, max_size - idx, "%-31s ", mb->name);
        idx += co_snprintf(buf + idx, max_size - idx, "%8u|%-4u ", mb->total - mb->size, mb->mail_count);
        idx += co_snprintf(buf + idx, max_size - idx, "%-8u ", mb->total);
        idx += co_snprintf(buf + idx, max_size - idx, "%-8u ", mb->wait_count);
        idx += co_snprintf(buf + idx, max_size - idx, "\r\n");
    }
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
    CO_LeaveCriticalSection();
    return idx;
}

static int PrintInfo(char *buf, int max_size)
{
    return _PrintInfo(buf, max_size, true);
}

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
    if (sem == nullptr)
        return nullptr;
    memset(sem, 0, sizeof(CO_Semaphore));
    sem->value = init_val;
    sem->list  = nullptr;
    int s      = name == nullptr ? 0 : strlen(name);
    if (s > sizeof(sem->name) - 1) s = sizeof(sem->name) - 1;
    memcpy(sem->name, name, s);
    sem->name[s] = '\0';
    // 加入信号列表
    CO_EnterCriticalSection();
    CM_NodeLink_Insert(&C_Static.semaphores,
                       CM_NodeLink_End(C_Static.semaphores),
                       &sem->link);
    CO_LeaveCriticalSection();
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
    CO_EnterCriticalSection();
    // 移出列表
    CM_NodeLink_Remove(&C_Static.semaphores, &sem->link);
    CO_LeaveCriticalSection();
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
    CO_EnterCriticalSection();
    sem->value += val;
    while (!CM_NodeLink_IsEmpty(sem->list)) {
        SemaphoreNode *n = CM_NodeLink_ToType(SemaphoreNode, link, CM_NodeLink_First(sem->list));
        if (n == nullptr || n->number > sem->value)
            break;
        sem->value -= n->number;
        n->isOk = true;
        // 移除等待列表
        CM_NodeLink_Remove(&sem->list, &n->link);
        // 开始执行
        CO_TCB *task = (CO_TCB *)n->task;
        // 清除等待标志
        task->execv_time = 0;
        task->timeout    = 0;
        // 独立栈从正在休眠的线程中唤醒
        uint16_t co_id = task->isSharedStack ? task->coroutine->co_id : GetSleepThread()->co_id;
        // 加入运行列表
        AddTaskList(task, task->priority);
        CO_LeaveCriticalSection();
        // 唤醒
        if (Inter.events->wake != nullptr)
            Inter.events->wake(co_id, Inter.events->object);
        CO_EnterCriticalSection();
    }
    CO_LeaveCriticalSection();
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
    CO_TCB *       task = c->idx_task;
    SemaphoreNode *n    = &task->wait_sem;
    bool           isOk = false;
    uint64_t       now  = Inter.GetMillisecond();
    do {
        // 检查信号值，没有就加入等待列表
        CO_EnterCriticalSection();
        if (sem->value >= val) {
            sem->value -= val;
            isOk = true;
        } else {
            n->isOk      = false;
            n->number    = val;
            n->task      = task;
            n->semaphore = sem;
            // 加入等待列表
            CM_NodeLink_Insert(&sem->list, CM_NodeLink_End(sem->list), &n->link);
            // 设置等待标志
            task->isWaitSem = true;
            // 计数
            sem->wait_count++;
        }
        CO_LeaveCriticalSection();
        if (isOk) {
            // 已经获取到信号，直接返回
            return true;
        }
        uint64_t tv = Inter.GetMillisecond() - now;
        if (tv >= (uint64_t)timeout) {
            _Yield(c, 0);
            return false;
        }
        // 计算剩余等待时间
        tv = timeout - tv;
        // 等待
        _Yield(c, tv);
        // 移除等待列表
        CO_EnterCriticalSection();
        if (n->isOk)
            isOk = true;
        CM_NodeLink_Remove(&sem->list, &n->link);
        task->isWaitSem = false;
        sem->wait_count--;
        CO_LeaveCriticalSection();
    } while (!isOk && Inter.GetMillisecond() - now < timeout);
    return isOk;
}

static Coroutine_Mutex CreateMutex(const char *name)
{
    CO_Mutex *mutex = (CO_Mutex *)Inter.Malloc(sizeof(CO_Mutex), __FILE__, __LINE__);
    if (mutex == nullptr) {
        if (Inter.events->Allocation)
            Inter.events->Allocation(__LINE__, sizeof(CO_Mutex), Inter.events->object);
        return nullptr;
    }
    memset(mutex, 0, sizeof(CO_Mutex));
    int s = name == nullptr ? 0 : strlen(name);
    if (s > sizeof(mutex->name) - 1) s = sizeof(mutex->name) - 1;
    memcpy(mutex->name, name, s);
    mutex->name[s] = '\0';
    // 加入列表
    CO_EnterCriticalSection();
    CM_NodeLink_Insert(&C_Static.mutexes, CM_NodeLink_End(C_Static.mutexes), &mutex->link);
    CO_LeaveCriticalSection();
    return mutex;
}

static void DeleteMutex(Coroutine_Mutex mutex)
{
    if (mutex == nullptr)
        return;
    // 移除列表
    CO_EnterCriticalSection();
    CM_NodeLink_Remove(&C_Static.mutexes, &mutex->link);
    CO_LeaveCriticalSection();
    Inter.Free(mutex, __FILE__, __LINE__);
    return;
}

static bool LockMutex(Coroutine_Mutex mutex, uint32_t timeout)
{
    CO_Thread *c = GetCurrentThread(-1);
    if (mutex == nullptr || c == nullptr || c->idx_task == nullptr)
        return false;
    CO_TCB * task = c->idx_task;
    bool     isOk = false;
    uint64_t now  = Inter.GetMillisecond();
    do {
        CO_EnterCriticalSection();
        if (mutex->owner == task) {
            // 已经锁定
            mutex->value++;
            isOk = true;
        } else if (mutex->owner == nullptr) {
            // 获取锁
            mutex->owner           = task;
            mutex->value           = 1;
            task->wait_mutex.mutex = mutex;
            isOk                   = true;
        } else {
            CO_TCB *owner = mutex->owner;
            if (owner->priority > task->priority) {
                // 优先级提升
                if (owner->isAddRunList || owner->isAddRunStandaloneList)
                    AddTaskList(owner, task->priority);
                else
                    owner->priority = task->priority;
            }
            // 等待锁
            CM_NodeLink_Insert(&mutex->list, CM_NodeLink_End(mutex->list), &task->wait_mutex.link);
            task->wait_mutex.mutex = mutex;
            task->isWaitMutex      = true;
            mutex->wait_count++;
        }
        CO_LeaveCriticalSection();
        if (isOk) return true;   // 获取锁成功
        uint64_t tv = Inter.GetMillisecond() - now;
        if (tv >= (uint64_t)timeout) {
            _Yield(c, 0);
            return false;
        }
        // 计算剩余等待时间
        tv = timeout - tv;
        // 等待
        _Yield(c, tv);
        // 移除等待列表
        CO_EnterCriticalSection();
        CM_NodeLink_Remove(&mutex->list, &task->wait_mutex.link);
        task->wait_mutex.mutex = nullptr;
        if (task->isWaitMutex)
            mutex->wait_count--;
        task->isWaitMutex = false;
        isOk              = mutex->owner == task;
        CO_LeaveCriticalSection();
    } while (!isOk && (Inter.GetMillisecond() - now) < timeout);
    return isOk;
}

static void UnlockMutex(Coroutine_Mutex mutex)
{
    CO_Thread *c = GetCurrentThread(-1);
    if (mutex == nullptr || c == nullptr || c->idx_task == nullptr)
        return;
    CO_TCB * task   = c->idx_task;
    bool     isWait = false;
    uint16_t co_id  = 0;
    CO_EnterCriticalSection();
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
                task = CM_NodeLink_ToType(CO_TCB,
                                          wait_mutex.link,
                                          CM_NodeLink_First(mutex->list));
                // 计数等待时间
                uint64_t tv  = task->execv_time - task->timeout;
                uint64_t now = Inter.GetMillisecond();
                if (now <= tv)
                    tv = 0;
                else
                    tv = now - tv;
                // 清除等待标志
                task->execv_time  = 0;
                task->timeout     = 0;
                task->isWaitMutex = false;
                mutex->owner      = task;
                mutex->value      = 1;
                mutex->wait_count--;
                // 加入等待列表
                AddTaskList(task, task->priority);
                // 获取最大等待时间
                if (mutex->max_wait_time < tv)
                    mutex->max_wait_time = tv;
                isWait = true;
                co_id  = task->isSharedStack ? task->coroutine->co_id : GetSleepThread()->co_id;
            }
        }
    }
    CO_LeaveCriticalSection();
    // 唤醒
    if (isWait && Inter.events->wake != nullptr)
        Inter.events->wake(co_id, Inter.events->object);
    return;
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
    if (c == nullptr) {
        if (Inter.events->Allocation)
            Inter.events->Allocation(__LINE__, sizeof(CO_Thread), Inter.events->object);
        return nullptr;
    }
    memset(c, 0, sizeof(CO_Thread));
    c->isRun      = true;
    c->start_time = Inter.GetMillisecond();
    c->ThreadId   = id;
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
    return Inter.Malloc(size, file, line);
}

static void Free(void *      ptr,
                 const char *file,
                 int         line)
{
    if (ptr) Inter.Free(ptr, file, line);
    return;
}

static void SetInter(const Coroutine_Inter *inter)
{
    Inter = *inter;
    if (inter->thread_count > 65535) Inter.thread_count = 65535;
    C_Static.coroutines = (CO_Thread **)Inter.Malloc(inter->thread_count * sizeof(CO_Thread *), __FILE__, __LINE__);
    memset(C_Static.coroutines, 0, inter->thread_count * sizeof(CO_Thread *));
    // 创建协程控制器
    for (uint16_t i = 0; i < inter->thread_count; i++) {
        C_Static.coroutines[i]        = Coroutine_Create((size_t)-1);
        C_Static.coroutines[i]->co_id = i;
    }
    // 创建任务映射表
    C_Static.tasks_map = (volatile CO_TCB **)Inter.Malloc(inter->thread_count * sizeof(CO_TCB *), __FILE__, __LINE__);
    memset((CO_TCB **)C_Static.tasks_map, 0, inter->thread_count * sizeof(CO_TCB *));
    // 默认堆栈大小
    C_Static.def_stack_size = coroutine_get_stack_default_size();
    return;
}

static const char *GetTaskName(Coroutine_TaskId taskId)
{
    return taskId == nullptr || taskId->name == nullptr ? "" : taskId->name;
}

static void _ASyncTask(void *obj)
{
    Coroutine_ASync a = (Coroutine_ASync)obj;
    if (a->func)
        a->ret = a->func(a->object);
    a->func = nullptr;   // 设置执行完成标志
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
    if (a == nullptr) {
        if (Inter.events->Allocation)
            Inter.events->Allocation(__LINE__, sizeof(CO_ASync), Inter.events->object);
        return nullptr;
    }
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
    Coroutine.WaitSemaphore(async->sem, 1, timeout);
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

static void FeedDog(uint32_t time)
{
    CO_Thread *c = GetCurrentThread(-1);
    if (c == nullptr || c->idx_task == nullptr)
        return;
    CO_EnterCriticalSection();
    CO_TCB *task = c->idx_task;
    // 从已有的列表中删除
    CM_NodeLink_Remove(&C_Static.watchdogs, &task->watchdog.link);
    // 设置超时时间
    if (time == 0)
        task->watchdog.expiration_time = 0;
    else
        task->watchdog.expiration_time = GetMillisecond() + time;
    if (task->watchdog.expiration_time) {
        // 添加到新的列表
        if (CM_NodeLink_IsEmpty(C_Static.watchdogs)) {
            CM_NodeLink_Init(&task->watchdog.link);
            C_Static.watchdogs = &task->watchdog.link;
        } else {
            CO_TCB *s = CM_NodeLink_ToType(CO_TCB, watchdog.link, CM_NodeLink_First(C_Static.watchdogs));
            CO_TCB *e = CM_NodeLink_ToType(CO_TCB, watchdog.link, CM_NodeLink_End(C_Static.watchdogs));
            if (s->watchdog.expiration_time > task->watchdog.expiration_time) {
                // 添加到头
                CM_NodeLink_Insert(&C_Static.watchdogs, CM_NodeLink_End(C_Static.watchdogs), &task->watchdog.link);
                C_Static.watchdogs = &task->watchdog.link;
            } else if (e->watchdog.expiration_time <= task->watchdog.expiration_time) {
                // 添加到尾巴
                CM_NodeLink_Insert(&C_Static.watchdogs, CM_NodeLink_End(C_Static.watchdogs), &task->watchdog.link);
            } else {
                // 添加到中间
                CO_TCB *r = nullptr;
                CM_NodeLink_Foreach_Positive(CO_TCB, watchdog.link, C_Static.watchdogs, p)
                {
                    if (p->watchdog.expiration_time >= task->watchdog.expiration_time) {
                        r = p;
                        break;
                    }
                }
                CM_NodeLink_Insert(&C_Static.watchdogs, r->watchdog.link.up, &task->watchdog.link);
            }
        }
    }
    CO_LeaveCriticalSection();
    return;
}

static void SetDefaultStackSize(uint32_t size)
{
    C_Static.def_stack_size = size;
}

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

const _Coroutine Coroutine = {
    SetInter,
    Coroutine_AddTask,
    Coroutine_GetTaskId,
    GetCurrentCoroutineIdx,
    GetThreadId,
    Coroutine_Yield,
    Coroutine_YieldTimeOut,
    Coroutine_RunTick,
    MakeMessage,
    DeleteMessage,
    CreateMailbox,
    DeleteMailbox,
    SendMail,
    ReceiveMail,
    PrintInfo,
    CreateSemaphore,
    DeleteSemaphore,
    GiveSemaphore,
    WaitSemaphore,
    CreateMutex,
    DeleteMutex,
    LockMutex,
    UnlockMutex,
    GetMillisecond,
    Malloc,
    Free,
    GetTaskName,
    ASync,
    ASyncWait,
    ASyncGetResultAndDelete,
    GetUniversal,
    FeedDog,
    SetDefaultStackSize,
};
