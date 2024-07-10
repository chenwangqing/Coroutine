
#include <stdlib.h>
#include "Coroutine.h"

#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/file.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/timeb.h>
#include <sys/times.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <asm-generic/ioctl.h>
#include <termios.h>
#include <linux/serial.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/tcp.h>
#include <semaphore.h>

typedef struct
{
    pthread_mutex_t obj;
} Mutex_t;

struct
{
    int32_t used;
    int32_t max_used;
} memory;

void *critical_section        = nullptr;
void *memory_critical_section = nullptr;

static void *__CreateLock(void)
{
    Mutex_t *lock = new Mutex_t();
    memset(lock, 0, sizeof(*lock));
    pthread_mutex_init(&lock->obj, nullptr); /* 动态初始化，	成功返回0，失败返回非0 */
    return lock;
}

static void __Lock(void *obj)
{
    Mutex_t *lock = (Mutex_t *)(obj);
    pthread_mutex_lock((pthread_mutex_t *)&(lock->obj));
    return;
}

static void __UnLock(void *obj)
{
    Mutex_t *lock = (Mutex_t *)(obj);
    pthread_mutex_unlock((pthread_mutex_t *)&(lock->obj));
    return;
}

static void _Lock(const char *file, int line)
{
    __Lock(critical_section);
    return;
}

static void _Unlock(const char *file, int line)
{
    __UnLock(critical_section);
    return;
}

static void _Free(void *ptr, const char *file, int line)
{
    size_t *p = (size_t *)ptr;
    p--;
    __Lock(memory_critical_section);
    memory.used -= (*p) + sizeof(size_t);
    __UnLock(memory_critical_section);
    free(p);
    return;
}

static void *_Malloc(size_t size, const char *file, int line)
{
    size_t *ptr = (size_t *)malloc(size + sizeof(size_t));
    *ptr        = size;
    __Lock(memory_critical_section);
    memory.used += size + sizeof(size_t);
    if (memory.used > memory.max_used)
        memory.max_used = memory.used;
    __UnLock(memory_critical_section);
    return ptr + 1;
}

static uint64_t GetMillisecond()
{
    struct timeval ts = {0};
    gettimeofday(&ts, NULL);
    return ts.tv_sec * 1000 + (ts.tv_usec / 1000);
}

static size_t GetThreadId(void)
{
    static __thread pid_t cached_tid = 0;
    if (cached_tid == 0)
        cached_tid = gettid();
    return (size_t)cached_tid;
}

void Sleep(uint32_t time)
{
    if (time == 0) {
        sched_yield();
        return;
    }
    uint32_t sec = time / 1000;
    uint32_t us  = (time % 1000) * 1000;
    usleep(us);
    sleep(sec);
    return;
}

static void Coroutine_WatchdogTimeout(void *object, Coroutine_TaskId taskId, const char *name)
{
    // return;
    while (true) {
        printf("\nWatchdogTimeout: %p %s\n", taskId, name);
        Sleep(1000);
    }
}

#define MAX_THREADS 4

class IdleNode {
public:
    pthread_cond_t  g_cond;
    pthread_mutex_t g_mutex;
    volatile bool   isWait = false;

    IdleNode()
    {
        pthread_cond_init(&g_cond, NULL);
        pthread_mutex_init(&g_mutex, NULL);
        return;
    }

    void Idle(uint32_t time)
    {
        isWait = true;
        struct timeval  now;
        struct timespec outtime;
        gettimeofday(&now, NULL);
        now.tv_sec += time / 1000;
        now.tv_usec += (time % 1000) * 1000;
        if (now.tv_usec >= 1000000) {
            now.tv_sec++;
            now.tv_usec -= 1000000;
        }
        outtime.tv_sec  = now.tv_sec;
        outtime.tv_nsec = now.tv_usec * 1000;
        pthread_mutex_lock(&g_mutex);
        pthread_cond_timedwait(&g_cond, &g_mutex, &outtime);
        pthread_mutex_unlock(&g_mutex);
        isWait = false;
        return;
    }

    void WeakUp()
    {
        pthread_mutex_lock(&g_mutex);
        if (isWait)
            pthread_cond_signal(&g_cond);
        pthread_mutex_unlock(&g_mutex);
        return;
    }
};

static IdleNode *idle_node[MAX_THREADS];

static Coroutine_Events events = {
    nullptr,
    [](void *object) -> void {
        sched_yield();
        return;
    },
    nullptr,
    [](uint32_t time, void *object) -> void {
        int idx = Coroutine.GetCurrentCoroutineIdx();
        idle_node[idx]->Idle(time);
        return;
    },
    [](uint16_t co_id, void *object) -> void {
        idle_node[co_id]->WeakUp();
        return;
    },
    Coroutine_WatchdogTimeout,
    [](void *object, Coroutine_TaskId taskId, const char *name) -> void {
        printf("Stack Error: %p %s\n", taskId, name);
        while (true)
            Sleep(1000);
    },
};

static const Coroutine_Inter Inter = {
    MAX_THREADS,
    _Lock,
    _Unlock,
    _Malloc,
    _Free,
    GetMillisecond,
    GetThreadId,
    &events,
};

const Coroutine_Inter *GetInter(void)
{
    critical_section        = __CreateLock();
    memory_critical_section = __CreateLock();
    for (int i = 0; i < MAX_THREADS; i++) {
        idle_node[i] = new IdleNode();
    }
    return &Inter;
}

void RunTask(void *(*func)(void *arg), void *arg)
{
    // 创建线程
    pthread_t      tmp;
    int            err = 0;
    pthread_attr_t attr;                                           // 线程属性
    pthread_attr_init(&attr);                                      // 初始化线程属性
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);   // 设置线程属性 分离线程 可自动释放资源
    pthread_attr_setstacksize(&attr, 32 << 10);                    // 一个线程内存设置为32KB
    err = pthread_create(&tmp, NULL, func, arg);
    pthread_attr_destroy(&attr);
    (void)err;
    return;
}

void PrintMemory(void)
{
    printf("Memory used: %d bytes, max used: %d bytes\n", memory.used, memory.max_used);
    return;
}
