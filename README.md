# 协程

## 介绍

使用 setjmp 和 longjmp 实现的协程。实现共享栈拷贝协程和独立栈协程两个类型的协程

方便再各平台移植使用

在单片机中可成为一个简易的操作系统使用

## 共享和独立

| 类型   | 栈切换模式   | 协程切换速度 | 栈内存使用 | 栈内存分配 | 移植性             |
| ------ | ------------ | ------------ | ---------- | ---------- | ------------------ |
| 共享栈 | 内存拷贝     | 慢           | 低         | 动态分配   | 好，几乎无需改动   |
| 独立栈 | 栈寄存器切换 | 快           | 高         | 固定分配   | 差，需要修改代码 |

## 使用示例

```c
// 协程事件
static Coroutine_Events events = {
    nullptr,
    [](void *object) -> void {
        SwitchToThread();// 切换线程
        return;
    },
    nullptr,
    [](uint32_t time, void *object) -> void {
        // 模拟休眠
        return;
    },
    [](uint16_t co_id, void *object) -> void {
        // 唤醒休眠
        return;
    },
    Coroutine_WatchdogTimeout,
};
// 协程接口
static const Coroutine_Inter Inter = {
    3,// 3 个线程
    _Lock,
    _Unlock,
    _Malloc,
    _Free,
    GetMillisecond,
    GetThreadId,
    &events,
};

// 注册协程
Coroutine.SetInter(&Inter);
// 创建信号量
sem1 = Coroutine.CreateSemaphore("sem1", 0);
// 添加任务
Coroutine_TaskAttribute atr;
memset(&atr, 0, sizeof(Coroutine_TaskAttribute));
atr.co_idx        = -1;             // 自动分配协程控制器
atr.stack_size    = 1024 * 16;      // 分配16KB独立栈空间
atr.pri           = TASK_PRI_NORMAL;// 默认优先级
atr.isSharedStack = false;          // 使用独立栈
Coroutine.AddTask(Task1, nullptr, "Task1", &atr);

// 运行协程的线程
DWORD WINAPI ThreadProc(LPVOID lpParam)
{
    while (true)
        Coroutine.RunTick();
    return 0;
}
```

## 独立栈协程移植

在 Coroutine.Platform 文件里需要实现函数：coroutine_enter_task
```c
void coroutine_enter_task(void *func, void *arg, int *stack)
{
#ifdef WIN32 // Windows x86
    __asm {
        // 1. 获取参数到寄存器
        mov eax, dword ptr[func]
        mov ebx, dword ptr[arg]
        mov ecx, dword ptr[stack]
        // 3. 切换栈
        mov esp, ecx
        // 4. 设置函数参数
        push ebx
        // 5. 调用函数
        call eax
    }
#else
    // TODO: 其他平台的实现
    #error "Coroutine.Platform: Unsupported platform!"
#endif
}
```
