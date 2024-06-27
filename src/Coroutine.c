
#include "Coroutine.h"
#ifdef WIN32
typedef uint32_t jmp_buf[6];
#else
#include <setjmp.h>
#endif

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
    return;
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
        mov ebx, dword ptr[buf]
        mov esi, dword ptr[ebx + 4]
        mov edi, dword ptr[ebx + 8]
        mov esp, dword ptr[ebx + 12]
        mov ebp, dword ptr[ebx + 16]
        mov ecx, dword ptr[ebx + 20]
        mov ebx, dword ptr[ebx + 0]
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
    bool                isOk;      // 等待成功
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
    uint32_t          wait_count;   // 等待数量
    CM_NodeLink_t     link;         // _CO_Semaphore
};

/**
 * @brief    协程节点
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-15
 */
struct _CO_TCB
{
    jmp_buf        env;              // 环境
    uint64_t       execv_time;       // 执行时间
    uint32_t       timeout;          // 超时时间
    int *          stack;            // 栈缓存
    int *          p_stack;          // 栈
    uint32_t       stack_len;        // 当前栈长度
    uint32_t       stack_max;        // 最大长度
    uint32_t       stack_alloc;      // 分配长度
    uint32_t       isDel : 1;        // 是否删除
    uint32_t       isRun : 1;        // 正在运行
    uint32_t       isWaitMail : 1;   // 正在等待邮件
    uint32_t       isWaitSem : 1;    // 等待信号量
    uint32_t       isFirst : 1;      // 首次
    Coroutine_Task func;             // 执行
    void *         obj;              // 执行参数
    uint64_t       run_time;         // 运行时间
    CO_Thread *    coroutine;        // 父节点
    int *          s, *p;            // 临时变量
    char *         name;             // 名称


    SemaphoreNode wait_sem;    // 信号等待节点
    MailWaitNode  wait_mail;   // 邮件等待节点

    struct _CO_TCB *next;
};

/**
 * @brief    协程线程
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-15
 */
struct _CO_Thread
{
    jmp_buf          env;               // 环境
    uint64_t         start_time;        // 启动时间
    uint64_t         run_time;          // 运行时间
    uint64_t         task_start_time;   // 任务开始时间
    uint32_t         isRun : 1;         // 运行
    volatile int *   stack;             // 栈
    CO_TCB *         tasks;             // 任务列表
    CO_TCB *         next_task;         // 下一个任务
    CO_TCB *         idx_task;          // 当前任务
    Coroutine_Events events;            // 事件
    char             name[32];          // 名称

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
    CM_NodeLinkList_t mails;        // 邮件列表
    CM_NodeLinkList_t waits;        // 等待列表 _MailWaitNode
    CM_NodeLink_t     link;         // _CO_Mailbox
};

static CO_TCB *        __C_NODE = nullptr;   // 用于获取堆上的相对地址
static Coroutine_Inter Inter;                // 外部接口

static struct
{
    CM_NodeLinkList_t threads;      // 协程控制器列表
    CM_NodeLinkList_t semaphores;   // 信号列表
    CM_NodeLinkList_t mailboxes;    // 邮箱列表
} C_Static;

#define CO_Lock()                       \
    do {                                \
        Inter.Lock(__FILE__, __LINE__); \
    } while (false);

#define CO_UnLock()                       \
    do {                                  \
        Inter.Unlock(__FILE__, __LINE__); \
    } while (false);

static void DeleteMessage(Coroutine_MailData *dat);

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
    // 释放堆栈
    Inter.Free(t->stack, __FILE__, __LINE__);
    // 释放实例
    Inter.Free(t, __FILE__, __LINE__);
    return;
}

/**
 * @brief    获取下一个任务
 * @param    coroutine      协程实例
 * @return   CO_Thread*
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-15
 */
static CO_TCB *GetNextTask(CO_Thread *coroutine, uint64_t ts, uint64_t *min_ts)
{
    if (coroutine->tasks == nullptr || coroutine->isRun == 0)
        return nullptr;
    CO_TCB *p = nullptr;
    if (coroutine->next_task == nullptr)
        p = coroutine->tasks;
    else
        p = coroutine->next_task;
    CO_TCB *up = nullptr;
    while (p != nullptr) {
        if (p->isDel) {
            CO_TCB *t = p;
            p         = p->next;
            if (t == coroutine->tasks)
                coroutine->tasks = p;
            else if (up == nullptr)
                continue;
            else
                up->next = p;
            DeleteTask(t);
            continue;
        }
        bool isTimeOut = p->execv_time == 0 || p->execv_time <= ts;
        if (p->isRun && isTimeOut)
            break;
        if (p->isRun && p->execv_time < *min_ts)
            *min_ts = p->execv_time;
        up = p;
        p  = p->next;
    }
    if (p == nullptr)
        coroutine->next_task = nullptr;
    else
        coroutine->next_task = p->next;
    return p;
}

/**
 * @brief    回收垃圾
 * @param    n
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-16
 */
static void rubbishRecycling(CO_Thread *coroutine, CO_TCB *n, uint64_t ts)
{
    if (n == nullptr)
        return;

    return;
}

static void _enter_into(CO_TCB *n)
{
    volatile int __stack = 0x44332211;
    // 获取栈起始指针
    if (n->coroutine->stack == nullptr)
        n->coroutine->stack = (int *)&__stack;   // 不能在后面创建局部变量
    else if (n->coroutine->stack != (int *)&__stack)
        return;   // 栈错误,栈被迁移
    // 保存环境
    volatile static func_setjmp_t _c_setjmp = setjmp;   // 防止setjmp内联
    int                           ret       = _c_setjmp(n->coroutine->env);
    if (ret == 0) {
        if (n->isFirst) {
            // 设置已运行标志
            n->isFirst = false;
            // 第一次运行
            n->func(n->coroutine, n->obj);
            // 设置删除标志
            n->isDel = true;
        } else {
            // 回到 SaveStack.setjmp
            longjmp(n->env, ((size_t)n - (size_t)__C_NODE));
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
    volatile uint64_t min_ts   = UINT64_MAX;
    volatile uint32_t flag2    = 0x55667788;
    volatile bool     isHead   = coroutine->next_task == nullptr;
    coroutine->task_start_time = Inter.GetMillisecond();
    // 获取下一个任务
    CO_Lock();
    rubbishRecycling(coroutine, n, coroutine->task_start_time);
    n = GetNextTask(coroutine, coroutine->task_start_time, &min_ts);
    CO_UnLock();
    coroutine->idx_task = n;
    if (n == nullptr) {
        // 运行空闲任务
        uint64_t sleep_time = min_ts > coroutine->task_start_time ? min_ts - coroutine->task_start_time : 1;
        if (coroutine->events.Idle != nullptr && isHead)
            coroutine->events.Idle(coroutine, sleep_time, coroutine->events.object);
        return;
    }
    _enter_into(n);
    // 记录运行时间
    uint64_t ts = Inter.GetMillisecond();
    uint64_t tv = ts <= coroutine->task_start_time ? 0 : ts - coroutine->task_start_time;
    coroutine->run_time += tv;
    n->run_time += tv;
    coroutine->idx_task = nullptr;
    // 周期事件
    if (coroutine->events.Period != nullptr)
        coroutine->events.Period(coroutine, n, coroutine->events.object);
    (void)flag1;
    (void)flag2;
    return;
}

/**
 * @brief    上下文切换
 * @param    coroutine
 * @param    timeout
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-31
 */
static void ContextWwitch(volatile CO_TCB *n)
{
    volatile int __mem = 0x11223344;   // 利用局部变量获取堆栈寄存器值
    // 保存栈数据
    n->p_stack   = (int *)&__mem - 4;   // 获取栈结尾
    n->stack_len = n->coroutine->stack - n->p_stack;
    if (n->stack_len > n->stack_alloc) {
        if (n->stack != nullptr)
            Inter.Free(n->stack, __FILE__, __LINE__);
        n->stack_alloc = ALIGN(n->stack_len, 128) * 128;   // 按512字节分配，避免内存碎片
        n->stack       = (int *)Inter.Malloc(n->stack_alloc * sizeof(int), __FILE__, __LINE__);
    }
    if (n->stack_len > n->stack_max) n->stack_max = n->stack_len;
    if (n->stack == nullptr) {
        // 内存分配错误
        n->isDel = true;
        if (n->coroutine->events.Allocation != nullptr)
            n->coroutine->events.Allocation(n->coroutine,
                                            __LINE__,
                                            n->stack_len,
                                            n->coroutine->events.object);
        return;
    } else
        memcpy((char *)n->stack, (char *)n->p_stack, n->stack_len * sizeof(int));
    volatile static func_setjmp_t _c_setjmp = setjmp;
    // 保存环境,回到调度器
    int ret = _c_setjmp(n->env);
    if (ret == 0)
        longjmp(n->coroutine->env, 1);
    n = (CO_TCB *)((size_t)ret + (size_t)__C_NODE);
    // 恢复堆栈内容
    n->p = n->p_stack + n->stack_len - 1;
    n->s = n->stack + n->stack_len - 1;
    while (n->s >= n->stack)   // 限制范围
        *(n->p)-- = *(n->s)--;
    return;
}

/**
 * @brief    转交控制权
 * @param    coroutine      线程实例
 * @param    timeout        超时
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-15
 */
static void _Yield(CO_Thread *coroutine, uint32_t timeout)
{
    CO_TCB *n = coroutine->idx_task;
    if (n == nullptr)
        return;
    if (timeout > 0)
        n->execv_time = Inter.GetMillisecond() + timeout;
    else
        n->execv_time = 0;
    n->timeout = timeout;
    // 保存堆栈数据
    ContextWwitch(n);
    return;
}

/**
 * @brief    结束任务
 * @param    taskId         任务id
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-16
 */
static void _task_exit(Coroutine_TaskId taskId, bool isEx)
{
    CO_TCB *n = (CO_TCB *)taskId;
    if (n == nullptr || n->coroutine == nullptr)
        return;
    // 设置结束标志
    n->isDel = true;
    if (isEx)
        return;
    // 跳转
    CO_Thread *coroutine = n->coroutine;
    longjmp(coroutine->env, 1);
    return;
}

/**
 * @brief    运行协程(一次运行一个任务)
 * @param    c              协程实例
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-17
 */
static bool Coroutine_RunTick(Coroutine_Handle c)
{
    CO_Thread *coroutine = (CO_Thread *)c;
    if (coroutine == nullptr)
        return false;
    _Task(coroutine);
    return coroutine->tasks != nullptr;
}

/**
 * @brief    删除协程
 * @param    c              协程实例
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-16
 */
static void Coroutine_Delete(Coroutine_Handle c)
{
    CO_Thread *coroutine = (CO_Thread *)c;
    if (coroutine == nullptr)
        return;
    CO_Lock();
    // 删除任务
    CO_TCB *n = coroutine->tasks;
    while (n != nullptr) {
        CO_TCB *t = n;
        n         = n->next;
        DeleteTask(t);
    }
    CM_NodeLink_Remove(&C_Static.threads, &coroutine->link);
    CO_UnLock();
    Inter.Free(coroutine, __FILE__, __LINE__);
    return;
}

static Coroutine_TaskId AddTask(Coroutine_Handle c,
                                Coroutine_Task   func,
                                void *           pars,
                                bool             isEx,
                                const char *     name)
{
    if (c == nullptr || func == nullptr)
        return nullptr;
    CO_Thread *coroutine = (CO_Thread *)c;
    CO_TCB *   n         = (CO_TCB *)Inter.Malloc(sizeof(CO_TCB), __FILE__, __LINE__);
    if (n == nullptr) {
        if (coroutine->events.Allocation != nullptr)
            coroutine->events.Allocation(coroutine, __LINE__, sizeof(CO_TCB), coroutine->events.object);
        return nullptr;
    }
    memset(n, 0, sizeof(CO_TCB));
    n->func        = func;
    n->obj         = pars;
    n->stack       = nullptr;
    n->coroutine   = coroutine;
    n->isRun       = true;
    n->isFirst     = true;
    n->next        = nullptr;
    n->stack       = (int *)Inter.Malloc(128 * sizeof(int), __FILE__, __LINE__);   // 预分配 512 字节
    n->stack_alloc = 128;
    if (name != nullptr && name[0] != '\0') {
        n->name = (char *)Inter.Malloc(32, __FILE__, __LINE__);
        if (n->name == nullptr) {
            Inter.Free(n, __FILE__, __LINE__);
            if (coroutine->events.Allocation != nullptr)
                coroutine->events.Allocation(coroutine, __LINE__, sizeof(CO_TCB), coroutine->events.object);
            return nullptr;
        }
        int s = strlen(name);
        if (s > 31) s = 31;
        memcpy(n->name, name, s + 1);
    }
    if (isEx) CO_Lock();
    if (coroutine->tasks == nullptr)
        coroutine->tasks = n;
    else {
        n->next          = coroutine->tasks;
        coroutine->tasks = n;
    }
    if (isEx) CO_UnLock();
    // 唤醒
    if (coroutine->events.wake != nullptr)
        coroutine->events.wake(coroutine, coroutine->events.object);
    return n;
}

/**
 * @brief    添加协程任务
 * @param    c              协程实例
 * @param    func           执行函数
 * @param    pars           执行参数
 * @return   int            任务id 0：创建失败
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-15
 */
static Coroutine_TaskId Coroutine_AddTask(Coroutine_Handle c,
                                          Coroutine_Task   func,
                                          void *           pars,
                                          const char *     name)
{
    return AddTask(c, func, pars, true, name);
}

/**
 * @brief    获取当前任务id
 * @param    c              协程实例
 * @return   Coroutine_TaskId
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-16
 */
static Coroutine_TaskId Coroutine_GetTaskId(Coroutine_Handle c)
{
    CO_Thread *coroutine = (CO_Thread *)c;
    return coroutine->idx_task;
}

/**
 * @brief    转交控制权
 * @param    coroutine      协程实例
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-15
 */
static void Coroutine_Yield(Coroutine_Handle coroutine)
{
    _Yield((CO_Thread *)coroutine, 0);
    return;
}

/**
 * @brief    转交控制权
 * @param    coroutine      线程实例
 * @param    timeout        超时 0：不超时
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-15
 */
static void Coroutine_YieldTimeOut(Coroutine_Handle coroutine, uint32_t timeout)
{
    _Yield((CO_Thread *)coroutine, timeout);
    return;
}

static void Coroutine_Suspend(Coroutine_TaskId taskId)
{
    CO_TCB *n = (CO_TCB *)taskId;
    if (taskId == nullptr || n->coroutine == nullptr)
        return;
    uint64_t ts = Inter.GetMillisecond();
    CO_Lock();
    n->isRun = false;
    if (n->execv_time > 0 && n->execv_time < ts)
        n->timeout = ts - n->execv_time;   // 计算剩余时间
    CO_UnLock();
    return;
}

/**
 * @brief    恢复
 * @param    taskId         任务id
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-16
 */
static void Resume(Coroutine_TaskId taskId, bool isEx)
{
    if (taskId == nullptr)
        return;
    CO_TCB *   n         = (CO_TCB *)taskId;
    CO_Thread *coroutine = n->coroutine;
    if (isEx) CO_Lock();
    n->isRun = true;
    if (n->timeout > 0)
        n->execv_time = Inter.GetMillisecond() + n->timeout;   // 时间继续
    if (isEx) CO_UnLock();
    // 唤醒
    if (coroutine->events.wake != nullptr)
        coroutine->events.wake(coroutine, coroutine->events.object);
    return;
}

static void Coroutine_Resume(Coroutine_TaskId taskId)
{
    Resume(taskId, false);
}

static void Coroutine_ResumeEx(Coroutine_TaskId taskId)
{
    Resume(taskId, true);
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
    CO_Lock();
    CM_NodeLink_Insert(&C_Static.mailboxes, CM_NodeLink_End(C_Static.mailboxes), &mb->link);
    CO_UnLock();
    return mb;
}

static void DeleteMailbox(Coroutine_Mailbox mb)
{
    if (mb == nullptr) return;
    CO_Lock();
    // 删除所有信息
    while (!CM_NodeLink_IsEmpty(mb->mails)) {
        Coroutine_MailData *md = CM_NodeLink_ToType(Coroutine_MailData, link, CM_NodeLink_First(mb->mails));
        CM_NodeLink_Remove(&mb->mails, &md->link);
        DeleteMessage(md);
    }
    // 移除邮箱列表
    CM_NodeLink_Remove(&C_Static.mailboxes, &mb->link);
    CO_UnLock();
    Inter.Free(mb, __FILE__, __LINE__);
    return;
}

static bool SendMail(Coroutine_Mailbox mb, Coroutine_MailData *data)
{
    if (mb == nullptr || data == nullptr)
        return false;
    CO_Lock();
    if (mb->size < data->size + sizeof(Coroutine_MailData)) {
        // 检查邮箱，是否有过期邮件
        uint64_t       now = Inter.GetMillisecond();
        CM_NodeLink_t *p   = CM_NodeLink_First(mb->mails);
        while (p != nullptr && mb->size < data->size + sizeof(Coroutine_MailData)) {
            Coroutine_MailData *md = CM_NodeLink_ToType(Coroutine_MailData, link, p);
            if (md->expiration_time <= now) {
                // 删除过期
                mb->size += md->size + sizeof(Coroutine_MailData);
                p = CM_NodeLink_Remove(&mb->mails, &md->link);
                if (p == mb->mails)
                    break;
                continue;
            }
            p = p->next;
            if (p == mb->mails)
                break;
        }
        if (mb->size < data->size + sizeof(Coroutine_MailData)) {
            CO_UnLock();
            return false;
        }
    }
    mb->size -= data->size + sizeof(Coroutine_MailData);
    // 检查等待列表
    if (!CM_NodeLink_IsEmpty(mb->waits)) {
        CM_NodeLink_t *p = CM_NodeLink_First(mb->waits);
        while (true) {
            MailWaitNode *n = CM_NodeLink_ToType(MailWaitNode, link, p);
            if (n->id_mask & data->eventId) {
                n->data = data;
                n->isOk = true;
                // 开始执行
                CO_TCB *task     = (CO_TCB *)n->task;
                task->execv_time = 0;
                task->timeout    = 0;
                data             = nullptr;
                break;
            }
            p = p->next;
            if (p == mb->waits)
                break;
        }
    }
    if (data == nullptr) {
        CO_UnLock();
        return true;
    }
    // 加入消息列表
    CM_NodeLink_Insert(&mb->mails, CM_NodeLink_End(mb->mails), &data->link);
    CO_UnLock();
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
    if (ret) mb->size += ret->size + sizeof(Coroutine_MailData);
    return ret;
}

static Coroutine_MailData *ReceiveMail(Coroutine_Handle  c,
                                       Coroutine_Mailbox mb,
                                       uint64_t          eventId_Mask,
                                       uint32_t          timeout)
{
    if (c == nullptr || c->idx_task == nullptr || mb == nullptr)
        return nullptr;
    CO_TCB * task = c->idx_task;
    uint64_t now  = Inter.GetMillisecond();
    CO_Lock();
    Coroutine_MailData *ret = GetMail(mb, eventId_Mask);
    CO_UnLock();
    if (ret) {
        _Yield(c, 0);
        return ret;
    }
    // 加入等待列表
    CO_Lock();
    task->isWaitMail = 1;   // 设置等待标志
    MailWaitNode *n  = &task->wait_mail;
    n->id_mask       = eventId_Mask;
    n->isOk          = false;
    n->task          = task;
    n->data          = nullptr;
    n->mailbox       = mb;
    CM_NodeLink_Insert(&mb->waits, CM_NodeLink_End(mb->waits), &n->link);
    mb->wait_count++;
    CO_UnLock();
    // 等待消息
    _Yield(c, timeout);
    // 取出消息
    CO_Lock();
    if (n->isOk) {
        ret     = n->data;
        n->data = nullptr;
        mb->size += ret->size + sizeof(Coroutine_MailData);
    }
    CM_NodeLink_Remove(&mb->waits, &n->link);
    task->isWaitMail = 0;   // 清除等待标志
    mb->wait_count--;
    CO_UnLock();
    return ret;
}

static int co_snprintf(char *buf, size_t size, const char *fmt, ...)
{
    if (buf == nullptr || size == 0)
        return 0;
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, size, fmt, args);
    va_end(args);
    if (len > size)
        len = size;
    return len;
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
    int idx = 0;
    // ---------------------------------- 标题 ----------------------------------
    idx += co_snprintf(buf + idx, max_size - idx, " SN  ");
    idx += co_snprintf(buf + idx, max_size - idx, "             Name              ");
    if (sizeof(size_t) == 4)
        idx += co_snprintf(buf + idx, max_size - idx, " TaskId  ");
    else
        idx += co_snprintf(buf + idx, max_size - idx, "    TaskId     ");
    if (sizeof(size_t) == 4)
        idx += co_snprintf(buf + idx, max_size - idx, " Func    ");
    else
        idx += co_snprintf(buf + idx, max_size - idx, "     Func      ");
    idx += co_snprintf(buf + idx, max_size - idx, "Status ");
    // 打印堆栈大小
    int max_stack = 13;
    int max_run   = 13;
    idx += co_snprintf(buf + idx, max_size - idx, "Stack ");
    for (int i = 5; i < max_stack; i++)
        buf[idx++] = ' ';
    // 打印运行时间
    idx += co_snprintf(buf + idx, max_size - idx, "Runtime ");
    for (int i = 7; i < max_run; i++)
        buf[idx++] = ' ';
    // 打印等待时间
    idx += co_snprintf(buf + idx, max_size - idx, " WaitTime  ");
    // 打印名称
    idx += co_snprintf(buf + idx, max_size - idx, " Name");
    idx += co_snprintf(buf + idx, max_size - idx, "\r\n");
    CO_Lock();
    CM_NodeLink_Foreach_Positive(CO_Thread, link, C_Static.threads, coroutine)
    {
        CO_TCB * p     = coroutine->tasks;
        uint64_t ts    = Inter.GetMillisecond();
        int      count = 0;
        // ---------------------------------- 内容 ----------------------------------
        p = coroutine->tasks;
        while (p != nullptr && idx < max_size) {
            char stack[32];
            int  len = co_snprintf(stack,
                                  sizeof(stack),
                                  "%u/%u",
                                  p->stack_len * sizeof(int),
                                  p->stack_max * sizeof(int));
            for (; len < max_stack; len++)
                stack[len] = ' ';
            stack[len] = '\0';
            char time[32];
            len = co_snprintf(time,
                              sizeof(time),
                              "%llu(%d%%)",
                              p->run_time,
                              (int)(p->run_time * 100 / (coroutine->run_time + 1)));
            for (; len < max_run; len++)
                time[len] = ' ';
            time[len] = '\0';
            idx += co_snprintf(buf + idx, max_size - idx, "%4d ", count + 1);
            if (idx >= max_size)
                break;
            idx += co_snprintf(buf + idx, max_size - idx, "%-32s", p->name == nullptr ? "" : p->name);
            if (idx >= max_size)
                break;
            idx += co_snprintf(buf + idx, max_size - idx, "%p ", p);
            if (idx >= max_size)
                break;
            idx += co_snprintf(buf + idx, max_size - idx, "%p ", p->func);
            if (idx >= max_size)
                break;
            idx += co_snprintf(buf + idx,
                               max_size - idx,
                               "   %s   ",
                               coroutine->idx_task == p ? "R" : (p->isWaitMail || p->isWaitSem) ? "W"
                                                                                                : "S");
            if (idx >= max_size)
                break;
            idx += co_snprintf(buf + idx, max_size - idx, "%s ", stack);
            if (idx >= max_size)
                break;
            idx += co_snprintf(buf + idx, max_size - idx, "%s ", time);
            if (idx >= max_size)
                break;
            idx += co_snprintf(buf + idx, max_size - idx, "%-10u ", (uint32_t)(p->execv_time == 0 || p->execv_time <= ts ? 0 : p->execv_time - ts));
            if (idx >= max_size)
                break;
            idx += co_snprintf(buf + idx, max_size - idx, "%s", p->name == nullptr ? "" : p->name);
            if (idx >= max_size)
                break;
            idx += co_snprintf(buf + idx, max_size - idx, "\r\n");
            p = p->next;
            count++;
        }
        uint64_t tv = Inter.GetMillisecond() - coroutine->start_time;
        if (tv == 0)
            tv = 1;
        int a = coroutine->run_time * 1000 / tv;
        idx += co_snprintf(buf + idx,
                           max_size - idx,
                           "Name: %-32s RunTime: %llu(%d.%02d%%) ms Task count: %u\r\n",
                           coroutine->name,
                           coroutine->run_time,
                           a / 100,
                           a % 100,
                           count);
    }
    // ----------------------------- 信号 -----------------------------
    idx += co_snprintf(buf + idx, max_size - idx, " SN  ");
    idx += co_snprintf(buf + idx, max_size - idx, "             Name              ");
    idx += co_snprintf(buf + idx, max_size - idx, " Value  ");
    idx += co_snprintf(buf + idx, max_size - idx, " Wait   ");
    idx += co_snprintf(buf + idx, max_size - idx, "\r\n");
    int sn = 0;
    CM_NodeLink_Foreach_Positive(CO_Semaphore, link, C_Static.semaphores, s)
    {
        idx += co_snprintf(buf + idx, max_size - idx, "%5d ", ++sn);
        idx += co_snprintf(buf + idx, max_size - idx, "%-32s", s->name);
        idx += co_snprintf(buf + idx, max_size - idx, "%-8u", s->value);
        idx += co_snprintf(buf + idx, max_size - idx, "%-8u", s->wait_count);
        idx += co_snprintf(buf + idx, max_size - idx, "\r\n");
    }
    // ----------------------------- 邮箱 -----------------------------
    idx += co_snprintf(buf + idx, max_size - idx, " SN  ");
    idx += co_snprintf(buf + idx, max_size - idx, "             Name              ");
    idx += co_snprintf(buf + idx, max_size - idx, " used   ");
    idx += co_snprintf(buf + idx, max_size - idx, " total  ");
    idx += co_snprintf(buf + idx, max_size - idx, " Wait   ");
    idx += co_snprintf(buf + idx, max_size - idx, "\r\n");
    sn = 0;
    CM_NodeLink_Foreach_Positive(CO_Mailbox, link, C_Static.mailboxes, mb)
    {
        idx += co_snprintf(buf + idx, max_size - idx, "%5d ", ++sn);
        idx += co_snprintf(buf + idx, max_size - idx, "%-32s", mb->name);
        idx += co_snprintf(buf + idx, max_size - idx, "%-8u", mb->total - mb->size);
        idx += co_snprintf(buf + idx, max_size - idx, "%-8u", mb->total);
        idx += co_snprintf(buf + idx, max_size - idx, "%-8u", mb->wait_count);
        idx += co_snprintf(buf + idx, max_size - idx, "\r\n");
    }
    CO_UnLock();
    return idx;
}

static int PrintInfo(char *buf, int max_size)
{
    return _PrintInfo(buf, max_size, true);
}

/**
 * @brief    结束任务
 * @param    taskId         任务id
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-16
 */
static void ExitTask(Coroutine_TaskId taskId)
{
    _task_exit(taskId, true);
    return;
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
    CO_Lock();
    CM_NodeLink_Insert(&C_Static.semaphores, CM_NodeLink_End(C_Static.semaphores), &sem->link);
    CO_UnLock();
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
    CO_Lock();
    // 移出列表
    CM_NodeLink_Remove(&C_Static.semaphores, &sem->link);
    CO_UnLock();
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
    CO_Lock();
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
        CO_TCB *task     = (CO_TCB *)n->task;
        task->execv_time = 0;
        task->timeout    = 0;
        CO_UnLock();
        // 唤醒
        if (task->coroutine->events.wake != nullptr)
            task->coroutine->events.wake(task->coroutine, task->coroutine->events.object);
        CO_Lock();
    }
    CO_UnLock();
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
static bool WaitSemaphore(Coroutine_Handle c, Coroutine_Semaphore _sem, uint32_t val, uint32_t timeout)
{
    if (val == 0)
        return true;
    CO_Semaphore *sem = (CO_Semaphore *)_sem;
    if (sem == nullptr || c == nullptr || c->idx_task == nullptr)
        return false;
    CO_TCB *       task = c->idx_task;
    SemaphoreNode *n    = &task->wait_sem;
    bool           isOk = false;
    // 检查信号值，没有就加入等待列表
    CO_Lock();
    if (sem->value >= val) {
        sem->value -= val;
        isOk = true;
    } else {
        n->isOk      = false;
        n->number    = val;
        n->task      = task;
        n->semaphore = sem;
        CM_NodeLink_Init(&n->link);
        // 加入等待列表
        CM_NodeLink_Insert(&sem->list, CM_NodeLink_End(sem->list), &n->link);
        // 设置等待标志
        task->isWaitSem = true;
        // 计数
        sem->wait_count++;
    }
    CO_UnLock();
    if (isOk) {
        _Yield(c, 0);
        return true;
    }
    // 等待
    _Yield(c, timeout);
    // 移除等待列表
    CO_Lock();
    if (n->isOk)
        isOk = true;
    CM_NodeLink_Remove(&sem->list, &n->link);
    task->isWaitSem = false;
    sem->wait_count--;
    CO_UnLock();
    return isOk;
}

/**
 * @brief    创建协程
 * @param    inter          通用接口
 * @param    isProtect      启用保护（加锁）
 * @return   Coroutine_Handle    nullptr 表示创建失败
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-15
 */
static Coroutine_Handle Coroutine_Create(const char *name, const Coroutine_Events *events)
{
    if (events == nullptr)
        return nullptr;
    CO_Thread *c = (CO_Thread *)Inter.Malloc(sizeof(CO_Thread),
                                             events->object,
                                             __FILE__,
                                             __LINE__);
    if (c == nullptr)
        return nullptr;
    memset(c, 0, sizeof(CO_Thread));
    c->isRun      = true;
    c->start_time = Inter.GetMillisecond(events->object);
    c->events     = *events;
    int s         = name == nullptr ? 0 : strlen(name);
    if (s > sizeof(c->name) - 1) s = sizeof(c->name) - 1;
    memcpy(c->name, name, s);
    c->name[s] = '\0';
    if (__C_NODE == nullptr)
        __C_NODE = (CO_TCB *)Inter.Malloc(1,
                                          events->object,
                                          __FILE__,
                                          __LINE__);
    CO_Lock();
    CM_NodeLink_Insert(&C_Static.threads, CM_NodeLink_End(C_Static.threads), &c->link);
    CO_UnLock();
    return c;
}

static uint64_t GetMillisecond()
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
    Inter.Free(ptr, file, line);
    return;
}

static void SetInter(const Coroutine_Inter *inter)
{
    Inter = *inter;
    return;
}

static const char *GetTaskName(Coroutine_TaskId taskId)
{
    return taskId == nullptr || taskId->name == nullptr ? "" : taskId->name;
}

const _Coroutine Coroutine = {
    SetInter,
    Coroutine_Create,
    Coroutine_Delete,
    Coroutine_AddTask,
    Coroutine_GetTaskId,
    Coroutine_Yield,
    Coroutine_YieldTimeOut,
    Coroutine_RunTick,
    Coroutine_Suspend,
    Coroutine_Resume,
    MakeMessage,
    DeleteMessage,
    CreateMailbox,
    DeleteMailbox,
    SendMail,
    ReceiveMail,
    PrintInfo,
    ExitTask,
    CreateSemaphore,
    DeleteSemaphore,
    GiveSemaphore,
    WaitSemaphore,
    GetMillisecond,
    Malloc,
    Free,
    GetTaskName,
};
