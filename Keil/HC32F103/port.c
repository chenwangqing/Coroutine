#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "uart.h"
#include <stdlib.h>
#include <stdio.h>
#include "hc32f10x.h"
#include "Coroutine.h"
#include "nplog.h"
#include "snprintf.h"

static volatile uint64_t Millisecond_count = 0;
static int               uart_log_fd       = 0;
extern Coroutine_Mutex   lock_log;
#define DEBUG_MEMORY 1

void CriticalSection(bool en)
{
    static volatile int val = 0;
    __disable_irq();
    if (en)
        val++;
    else {
        val--;
        if (val == 0)
            __enable_irq();
    }
    return;
}

void SysTick_Handler(void)
{
    Millisecond_count++;
    return;
}

uint64_t GetMilliseconds(void)
{
    CriticalSection(true);
    uint64_t ts = Millisecond_count;
    CriticalSection(false);
    return ts;
}

static size_t GetThreadId(void)
{
    return 0;
}

static void _Lock(const char *file, int line)
{
    CriticalSection(true);
    return;
}

static void _Unlock(const char *file, int line)
{
    CriticalSection(false);
    return;
}

#if DEBUG_MEMORY
struct
{
    int64_t used;
    int64_t max_used;
} memory;
#endif

static void _Free(void *ptr, const char *file, int line)
{
    if (ptr == nullptr)
        return;
#if DEBUG_MEMORY
    size_t *p = (size_t *)ptr;
    p--;
    CriticalSection(true);
    memory.used -= (*p) + sizeof(size_t);
    CriticalSection(false);
    free(p);
#else
    free(ptr);
#endif
    return;
}

static void *_Malloc(size_t size, const char *file, int line)
{
#if DEBUG_MEMORY
    size_t *ptr = (size_t *)malloc(size + sizeof(size_t));
    *ptr        = size;
    CriticalSection(true);
    memory.used += size + sizeof(size_t);
    if (memory.used > memory.max_used)
        memory.max_used = memory.used;
    CriticalSection(false);
    return ptr + 1;
#else
    return malloc(size);
#endif
}

static Coroutine_Events events = {NULL};

static const Coroutine_Inter Inter = {
    1,
    _Lock,
    _Unlock,
    _Malloc,
    _Free,
    GetMilliseconds,
    GetThreadId,
    &events,
};

const Coroutine_Inter *GetInter(void)
{
    return &Inter;
}

static char log_buf[512];

void UART_LOG(const void *data, int lens)
{
    Coroutine.LockMutex(lock_log, UINT32_MAX);
    UART_Write(uart_log_fd, data, lens);
    Coroutine.UnlockMutex(lock_log);
    return;
}

static int _print(int level, const char *_file, int line, const char *fmt, ...)
{
    Coroutine.LockMutex(lock_log, UINT32_MAX);
    level = level < 0 ? 0 : (level > NPLOG_LEVEL_MAX ? NPLOG_LEVEL_MAX : level);

    char file[15 + 1];
    if (_file[0] == '[') {
        const char *p = _file + 1;
        while (*p && *p != ']')
            p++;
        int n = p - _file - 1;
        n &= 0x0F;
        memcpy(file, _file + 1, n);
        file[n] = '\0';
    } else {
        const char *p = _file;
        const char *s = p;
        while (*p) {
            char ch = *p++;
            if (ch == '\\' || ch == '/')
                s = p;
        }
        const char *e = p;
        while (e > _file && *e != '.')
            e--;
        int n = e - s;
        n &= 0x0F;
        memcpy(file, s, n);
        file[n] = '\0';
    }
    int       len   = 0;
    uint64_t  now   = GetMilliseconds();
    time_t    times = now / 1000;
    struct tm nowTime;
    Localtime(times, nowTime);
    len += ff_snprintf(log_buf + len, sizeof(log_buf) - len, "%llu|%02d %02d-%02d:%02d:%02d|", now, nowTime.tm_mon + 1, nowTime.tm_mday, nowTime.tm_hour, nowTime.tm_min, nowTime.tm_sec);
    len += ff_snprintf(log_buf + len, sizeof(log_buf) - len, "%d|%-15s|%04d|", level, file, line);
    va_list argptr;
    va_start(argptr, fmt);
    len += ff_vsnprintf(log_buf + len, sizeof(log_buf) - len, fmt, argptr);
    va_end(argptr);
    UART_LOG(log_buf, len);
    Coroutine.UnlockMutex(lock_log);
    return len;
}

static void _PrintArray(int level, const char *_file, int line, const void *data, int lens)
{
    Coroutine.LockMutex(lock_log, UINT32_MAX);
    level = level < 0 ? 0 : (level > NPLOG_LEVEL_MAX ? NPLOG_LEVEL_MAX : level);

    char file[15 + 1];
    if (_file[0] == '[') {
        const char *p = _file + 1;
        while (*p && *p != ']')
            p++;
        int n = p - _file - 1;
        n &= 0x0F;
        memcpy(file, _file + 1, n);
        file[n] = '\0';
    } else {
        const char *p = _file;
        const char *s = p;
        while (*p) {
            char ch = *p++;
            if (ch == '\\' || ch == '/')
                s = p;
        }
        const char *e = p;
        while (e > _file && *e != '.')
            e--;
        int n = e - s;
        n &= 0x0F;
        memcpy(file, s, n);
        file[n] = '\0';
    }
    int       len   = 0;
    uint64_t  now   = GetMilliseconds();
    time_t    times = now / 1000;
    struct tm nowTime;
    Localtime(times, nowTime);
    len += ff_snprintf(log_buf + len, sizeof(log_buf) - len, "%llu|%02d %02d-%02d:%02d:%02d|", now, nowTime.tm_mon + 1, nowTime.tm_mday, nowTime.tm_hour, nowTime.tm_min, nowTime.tm_sec);
    len += ff_snprintf(log_buf + len, sizeof(log_buf) - len, "%d|%-15s|%04d|", level, file, line);
    const uint8_t *p = (uint8_t *)data;
    for (int i = 0; i < lens && len < sizeof(log_buf) - 4; i++) {
        if ((i % 16) == 0 && lens > 16) {
            log_buf[len++] = '\r';
            log_buf[len++] = '\n';
        }
        len += ff_snprintf(log_buf + len, sizeof(log_buf) - len, "%02X ", p[i]);
    }
    len += ff_snprintf(log_buf + len, sizeof(log_buf) - len, "\r\n");
    UART_LOG(log_buf, len);
    Coroutine.UnlockMutex(lock_log);
    return;
}

NPLOG nplog = {_print, _PrintArray};

void PrintMemory(void)
{
    LOG_PRINTF("Memory used: %lld bytes, max used: %lld bytes", memory.used, memory.max_used);
    return;
}
