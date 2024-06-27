
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
    CM_NodeLinkList_t list;    // 等待列表 _SemaphoreNode
    uint32_t          value;   // 信号值
    CM_NodeLink_t     link;    // _CO_Semaphore
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
    jmp_buf          env;          // 环境
    uint64_t         start_time;   // 启动时间
    uint64_t         run_time;     // 运行时间
    uint32_t         isRun : 1;    // 运行
    volatile int *   stack;        // 栈
    CO_TCB *         tasks;        // 任务列表
    CO_TCB *         next_task;    // 下一个任务
    CO_TCB *         idx_task;     // 当前任务
    Coroutine_Events events;       // 事件
};

/**
 * @brief    
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-06-26
 */
struct _CO_Mailbox
{
    uint32_t          size;    // 邮箱大小
    CM_NodeLinkList_t mails;   // 邮件列表
    CM_NodeLinkList_t waits;   // 等待列表 _MailWaitNode
    CM_NodeLink_t     link;    // _CO_Mailbox
};

static CO_TCB *        __C_NODE = NULL;   // 用于获取堆上的相对地址
static Coroutine_Inter Inter;             // 外部接口

static struct
{
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
        if (t->wait_mail.data != NULL) {
            DeleteMessage(t->wait_mail.data);
            t->wait_mail.data = NULL;
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
    if (coroutine->tasks == NULL || coroutine->isRun == 0)
        return NULL;
    CO_TCB *p = NULL;
    if (coroutine->next_task == NULL)
        p = coroutine->tasks;
    else
        p = coroutine->next_task;
    CO_TCB *up = NULL;
    while (p != NULL) {
        if (p->isDel) {
            CO_TCB *t = p;
            p         = p->next;
            if (t == coroutine->tasks)
                coroutine->tasks = p;
            else if (up == NULL)
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
    if (p == NULL)
        coroutine->next_task = NULL;
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
    if (n == NULL)
        return;

    return;
}

static void _enter_into(CO_TCB *n)
{
    volatile int __stack = 0x44332211;
    // 获取栈起始指针
    if (n->coroutine->stack == NULL)
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
    volatile CO_TCB * n      = NULL;
    volatile uint64_t min_ts = UINT64_MAX;
    volatile uint64_t ts     = Inter.GetMillisecond();
    volatile bool     isHead = coroutine->next_task == NULL;
    // 获取下一个任务
    CO_Lock();
    rubbishRecycling(coroutine, n, ts);
    n = GetNextTask(coroutine, ts, &min_ts);
    CO_UnLock();
    coroutine->idx_task = n;
    if (n == NULL) {
        // 运行空闲任务
        uint64_t sleep_time = min_ts > ts ? min_ts - ts : 1;
        if (coroutine->events.Idle != NULL && isHead)
            coroutine->events.Idle(coroutine, sleep_time, coroutine->events.object);
        return;
    }
    _enter_into(n);
    // 记录运行时间
    ts = Inter.GetMillisecond() - ts;
    coroutine->run_time += ts;
    n->run_time += ts;
    coroutine->idx_task = NULL;
    // 周期事件
    if (coroutine->events.Period != NULL)
        coroutine->events.Period(coroutine, n, coroutine->events.object);
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
    if (n->stack_len > n->stack_max) {
        if (n->stack != NULL)
            Inter.Free(n->stack, __FILE__, __LINE__);
        n->stack_max = ALIGN(n->stack_len, 128) * 128;   // 按512字节分配，避免内存碎片
        n->stack     = (int *)Inter.Malloc(n->stack_max * sizeof(int), __FILE__, __LINE__);
    }
    if (n->stack == NULL) {
        // 内存分配错误
        n->isDel = true;
        if (n->coroutine->events.Allocation != NULL)
            n->coroutine->events.Allocation(n->coroutine,
                                            __LINE__,
                                            n->stack_len,
                                            n->coroutine->events.object);
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
    if (n == NULL)
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
    if (n == NULL || n->coroutine == NULL)
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
    if (coroutine == NULL)
        return false;
    _Task(coroutine);
    return coroutine->tasks != NULL;
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
    if (coroutine == NULL)
        return;
    CO_Lock();
    // 删除任务
    CO_TCB *n = coroutine->tasks;
    while (n != NULL) {
        CO_TCB *t = n;
        n         = n->next;
        DeleteTask(t);
    }
    Inter.Free(coroutine, __FILE__, __LINE__);
    return;
}

static Coroutine_TaskId AddTask(Coroutine_Handle c,
                                Coroutine_Task   func,
                                void *           pars,
                                bool             isEx)
{
    if (c == NULL || func == NULL)
        return NULL;
    CO_Thread *coroutine = (CO_Thread *)c;
    CO_TCB *   n         = (CO_TCB *)Inter.Malloc(sizeof(CO_TCB), __FILE__, __LINE__);
    if (n == NULL) {
        if (coroutine->events.Allocation != NULL)
            coroutine->events.Allocation(coroutine, __LINE__, sizeof(CO_TCB), coroutine->events.object);
        return NULL;
    }
    memset(n, 0, sizeof(CO_TCB));
    n->func      = func;
    n->obj       = pars;
    n->stack     = NULL;
    n->coroutine = coroutine;
    n->isRun     = true;
    n->isFirst   = true;
    n->next      = NULL;
    n->stack     = (int *)Inter.Malloc(128 * sizeof(int), __FILE__, __LINE__);   // 预分配 512 字节
    n->stack_max = 128;
    if (isEx) CO_Lock();
    if (coroutine->tasks == NULL)
        coroutine->tasks = n;
    else {
        n->next          = coroutine->tasks;
        coroutine->tasks = n;
    }
    if (isEx) CO_UnLock();
    // 唤醒
    if (coroutine->events.wake != NULL)
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
                                          void *           pars)
{
    return AddTask(c, func, pars, true);
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
    if (taskId == NULL || n->coroutine == NULL)
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
    if (taskId == NULL)
        return;
    CO_TCB *   n         = (CO_TCB *)taskId;
    CO_Thread *coroutine = n->coroutine;
    if (isEx) CO_Lock();
    n->isRun = true;
    if (n->timeout > 0)
        n->execv_time = Inter.GetMillisecond() + n->timeout;   // 时间继续
    if (isEx) CO_UnLock();
    // 唤醒
    if (coroutine->events.wake != NULL)
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
    Coroutine_MailData *dat = (Coroutine_MailData *)Inter.Malloc(sizeof(Coroutine_MailData) + size,
                                                                 __FILE__,
                                                                 __LINE__);
    if (dat == NULL) {
        return NULL;
    }
    dat->data = (char *)(dat + 1);
    if (data != NULL)
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
    if (dat == NULL)
        return;
    Inter.Free(dat, __FILE__, __LINE__);
    return;
}

static Coroutine_Mailbox CreateMailbox(uint32_t msg_max_size)
{
    CO_Mailbox *mb = (CO_Mailbox *)Inter.Malloc(sizeof(CO_Mailbox), __FILE__, __LINE__);
    if (mb == NULL) return NULL;
    memset(mb, 0, sizeof(CO_Mailbox));
    mb->size = msg_max_size;
    // 加入邮箱列表
    CO_Lock();
    CM_NodeLink_Insert(&C_Static.mailboxes, CM_NodeLink_End(C_Static.mailboxes), &mb->link);
    CO_UnLock();
    return mb;
}

static void DeleteMailbox(Coroutine_Mailbox mb)
{
    if (mb == NULL) return;
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
    if (mb == NULL || data == NULL)
        return false;
    CO_Lock();
    if (mb->size < data->size + sizeof(Coroutine_MailData)) {
        // 检查邮箱
        uint64_t       now = Inter.GetMillisecond();
        CM_NodeLink_t *p   = CM_NodeLink_First(mb->mails);
        while (p != NULL && mb->size < data->size + sizeof(Coroutine_MailData)) {
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
                data == NULL;
                break;
            }
            p = p->next;
            if (p == mb->waits)
                break;
        }
    }
    if (data == NULL) {
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
    Coroutine_MailData *ret = NULL;
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
    if (c == NULL || c->idx_task == NULL || mb == NULL)
        return NULL;
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
    n->data          = NULL;
    n->mailbox       = mb;
    CM_NodeLink_Insert(&mb->waits, CM_NodeLink_End(mb->waits), &n->link);
    CO_UnLock();
    // 等待消息
    _Yield(c, timeout);
    // 取出消息
    CO_Lock();
    if (n->isOk) {
        ret     = n->data;
        n->data = NULL;
        mb->size += ret->size + sizeof(Coroutine_MailData);
    }
    CM_NodeLink_Remove(&mb->waits, &n->link);
    task->isWaitMail = 0;   // 清除等待标志
    CO_UnLock();
    return ret;
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
static int _PrintInfo(Coroutine_Handle c, char *buf, int max_size, bool isEx)
{
    CO_Thread *coroutine = (CO_Thread *)c;
    if (buf == NULL || max_size <= 0)
        return 0;
    CO_Lock();
    CO_TCB * p     = coroutine->tasks;
    uint64_t ts    = Inter.GetMillisecond();
    int      idx   = 0;
    int      count = 0;
    // ---------------------------------- 标题 ----------------------------------
    snprintf(buf + idx, max_size - idx, " SN  ");
    idx += strlen(buf + idx);
    if (sizeof(size_t) == 4)
        snprintf(buf + idx, max_size - idx, " TaskId  ");
    else
        snprintf(buf + idx, max_size - idx, "    TaskId     ");
    idx += strlen(buf + idx);
    if (sizeof(size_t) == 4)
        snprintf(buf + idx, max_size - idx, " Func    ");
    else
        snprintf(buf + idx, max_size - idx, "     Func      ");
    idx += strlen(buf + idx);
    snprintf(buf + idx, max_size - idx, "Status ");
    idx += strlen(buf + idx);

    // 打印堆栈大小
    p             = coroutine->tasks;
    int max_stack = 0;
    int max_run   = 0;
    while (p != NULL) {
        char stack[32];
        int  len = snprintf(stack,
                           sizeof(stack),
                           "%u/%u",
                           p->stack_len * sizeof(int),
                           p->stack_max * sizeof(int));
        if (len > sizeof(stack) - 1)
            len = sizeof(stack) - 1;
        if (len > max_stack)
            max_stack = len;
        char time[32];
        len = snprintf(time,
                       sizeof(time),
                       "%llu(%d%%)",
                       p->run_time,
                       (int)(p->run_time * 100 / (coroutine->run_time + 1)));
        if (len > sizeof(time) - 1)
            len = sizeof(time) - 1;
        if (len > max_run)
            max_run = len;
        p = p->next;
    }
    snprintf(buf + idx, max_size - idx, "Stack ");
    idx += strlen(buf + idx);
    for (int i = 5; i < max_stack; i++)
        buf[idx++] = ' ';
    // 打印运行时间
    snprintf(buf + idx, max_size - idx, "Runtime ");
    idx += strlen(buf + idx);
    for (int i = 7; i < max_run; i++)
        buf[idx++] = ' ';
    // 打印等待时间
    snprintf(buf + idx, max_size - idx, " WaitTime  ");
    idx += strlen(buf + idx);
    // 打印名称
    snprintf(buf + idx, max_size - idx, " Name");
    idx += strlen(buf + idx);

    snprintf(buf + idx, max_size - idx, "\r\n");
    idx += 2;
    // ---------------------------------- 内容 ----------------------------------

    p = coroutine->tasks;
    while (p != NULL && idx < max_size) {
        char stack[32];
        int  len = snprintf(stack,
                           sizeof(stack),
                           "%u/%u",
                           p->stack_len * sizeof(int),
                           p->stack_max * sizeof(int));
        for (; len < max_stack; len++)
            stack[len] = ' ';
        stack[len] = '\0';
        char time[32];
        len = snprintf(time,
                       sizeof(time),
                       "%llu(%d%%)",
                       p->run_time,
                       (int)(p->run_time * 100 / (coroutine->run_time + 1)));
        for (; len < max_run; len++)
            time[len] = ' ';
        time[len] = '\0';

        snprintf(buf + idx, max_size - idx, "%4d ", count + 1);
        idx += strlen(buf + idx);
        if (idx >= max_size)
            break;
        snprintf(buf + idx, max_size - idx, "%p ", p);
        idx += strlen(buf + idx);
        if (idx >= max_size)
            break;
        snprintf(buf + idx, max_size - idx, "%p ", p->func);
        idx += strlen(buf + idx);
        if (idx >= max_size)
            break;
        snprintf(buf + idx,
                 max_size - idx,
                 "   %s   ",
                 coroutine->idx_task == p ? "R" : (p->isWaitMail || p->isWaitSem) ? "W"
                                                                                  : "S");
        idx += strlen(buf + idx);
        if (idx >= max_size)
            break;
        snprintf(buf + idx, max_size - idx, "%s ", stack);
        idx += strlen(buf + idx);
        if (idx >= max_size)
            break;
        snprintf(buf + idx, max_size - idx, "%s ", time);
        idx += strlen(buf + idx);
        if (idx >= max_size)
            break;
        snprintf(buf + idx, max_size - idx, "%-10u ", (uint32_t)(p->execv_time == 0 || p->execv_time <= ts ? 0 : p->execv_time - ts));
        idx += strlen(buf + idx);
        if (idx >= max_size)
            break;
        snprintf(buf + idx, max_size - idx, "%s", p->name == NULL ? "" : p->name);
        idx += strlen(buf + idx);
        if (idx >= max_size)
            break;
        snprintf(buf + idx, max_size - idx, "\r\n");
        idx += 2;
        p = p->next;
        count++;
    }
    uint64_t tv = Inter.GetMillisecond() - coroutine->start_time;
    if (tv == 0)
        tv = 1;
    int a = coroutine->run_time * 1000 / tv;
    snprintf(buf + idx,
             max_size - idx,
             "RunTime: %llu(%d.%02d%%) ms Task count: %u",
             coroutine->run_time,
             a / 100,
             a % 100,
             count);
    CO_UnLock();
    return idx;
}

static int PrintInfo(Coroutine_Handle c, char *buf, int max_size)
{
    return _PrintInfo(c, buf, max_size, true);
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
static Coroutine_Semaphore CreateSemaphore(uint32_t init_val)
{
    CO_Semaphore *sem = (CO_Semaphore *)Inter.Malloc(sizeof(CO_Semaphore),
                                                     __FILE__,
                                                     __LINE__);
    if (sem == NULL) {
        return NULL;
    }
    memset(sem, 0, sizeof(CO_Semaphore));
    sem->value = init_val;
    sem->list  = NULL;
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
    if (sem == NULL)
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
    if (sem == NULL || val == 0)
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
        if (task->coroutine->events.wake != NULL)
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
    if (sem == NULL || c == NULL || c->idx_task == NULL)
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
    CO_UnLock();
    return isOk;
}

/**
 * @brief    设置名称
 * @param    taskId         任务id
 * @param    name           名称
 * @param    isEx
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-09-09
 */
static void SetName(Coroutine_TaskId taskId, const char *name)
{
    if (taskId == NULL || name == NULL)
        return;
    int        name_len  = strlen(name) + 1;
    CO_TCB *   n         = (CO_TCB *)taskId;
    CO_Thread *coroutine = n->coroutine;

    char *str = Inter.Malloc(name_len,
                             coroutine->events.object,
                             __FILE__,
                             __LINE__);
    if (str == NULL) {
        if (coroutine->events.Allocation != NULL)
            coroutine->events.Allocation(coroutine, __LINE__, name_len, coroutine->events.object);
        return;
    }
    memcpy(str, name, name_len);
    CO_Lock();
    char *old = n->name;
    n->name   = str;
    CO_UnLock();
    if (old) Inter.Free(old, __FILE__, __LINE__);
    return;
}

/**
 * @brief    创建协程
 * @param    inter          通用接口
 * @param    isProtect      启用保护（加锁）
 * @return   Coroutine_Handle    NULL 表示创建失败
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-15
 */
static Coroutine_Handle Coroutine_Create(const Coroutine_Events *events)
{
    if (events == NULL)
        return NULL;
    CO_Thread *c = (CO_Thread *)Inter.Malloc(sizeof(CO_Thread),
                                             events->object,
                                             __FILE__,
                                             __LINE__);
    if (c == NULL)
        return NULL;
    memset(c, 0, sizeof(CO_Thread));
    c->isRun      = true;
    c->start_time = Inter.GetMillisecond(events->object);
    c->events     = *events;
    if (__C_NODE == NULL)
        __C_NODE = (CO_TCB *)Inter.Malloc(1,
                                          events->object,
                                          __FILE__,
                                          __LINE__);
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
    SetName,
    GetMillisecond,
    Malloc,
    Free,
};
