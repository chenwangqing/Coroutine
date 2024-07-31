
#include "Coroutine.h"
#include "nplog.h"
#include "Print.h"


// --------------------------------------------------------------------------------------
//                              |       输出        |
// --------------------------------------------------------------------------------------

static void output(const char *msg, int len)
{
    puts(msg);
    return;
}

// --------------------------------------------------------------------------------------
//                              |       格式化        |
// --------------------------------------------------------------------------------------


#ifndef CFG_LOG_LEVEL_STR
#define CFG_LOG_LEVEL_STR              \
    {                                  \
        "DBUG", "INFO", "WARN", "FAIL" \
    }
#endif

static const char *log_level_str[] = CFG_LOG_LEVEL_STR;

static void GetFileName(const char *_file, char *file_buff, size_t file_buff_size)
{
    size_t len        = strlen(_file) - 1;
    size_t file_start = 0, file_end = len + 1;
    while (len > 0) {
        if (_file[len] == '/' || _file[len] == '\\') {
            file_start = len + 1;
            break;
        }
        len--;
    }
    if (file_end < file_start)
        file_end = file_start;
    len        = file_end - file_start;
    len        = len > file_buff_size ? file_buff_size : len;
    file_start = file_end - len;
    memcpy(file_buff, _file + file_start, len);
    file_buff[len] = '\0';
    return;
}

#define PRINT_HEADER                                                                                                                                                   \
    level = level < 0 ? 0 : (level > NPLOG_LEVEL_MAX ? NPLOG_LEVEL_MAX : level);                                                                                       \
    if (info != NULL && level < info->level)                                                                                                                           \
        return 0;                                                                                                                                                      \
    const int mlen = 1024;                                                                                                                                             \
    int       idx  = 0;                                                                                                                                                \
    char      file_buff[15 + 1];                                                                                                                                       \
    uint64_t  now  = Coroutine.GetMillisecond();                                                                                                                       \
    char *    buff = (char *)Coroutine.Malloc(mlen, __FILE__, __LINE__);                                                                                               \
    if (buff == NULL)                                                                                                                                                  \
        return 0;                                                                                                                                                      \
    if (info == NULL)                                                                                                                                                  \
        GetFileName(_file, file_buff, 15);                                                                                                                             \
    else {                                                                                                                                                             \
        strncpy(file_buff, info->name, 15);                                                                                                                            \
        file_buff[15] = '\0';                                                                                                                                          \
    }                                                                                                                                                                  \
    time_t    times = now / 1000;                                                                                                                                      \
    struct tm nowTime;                                                                                                                                                 \
    Localtime(times, nowTime);                                                                                                                                         \
    idx += Print(buff + idx, mlen - idx, "%llu|%02d/%02d %02d:%02d:%02d|", now, nowTime.tm_mon + 1, nowTime.tm_mday, nowTime.tm_hour, nowTime.tm_min, nowTime.tm_sec); \
    idx += Print(buff + idx, mlen - idx, "%s|%-15s|%04d|%p|", log_level_str[level], file_buff, line, Coroutine.GetCurrentTaskId());

static int _print(const NPLOG_Info *info, int level, const char *_file, int line, const char *fmt, ...)
{
    PRINT_HEADER
    va_list argptr;
    va_start(argptr, fmt);
    idx += vPrint(buff + idx, mlen - idx, fmt, argptr);
    va_end(argptr);
    output(buff, idx);
    Coroutine.Free(buff, __FILE__, __LINE__);
    return idx;
}

static int _PrintArray(const NPLOG_Info *info, int level, const char *_file, int line, const void *data, int lens)
{
    PRINT_HEADER
    uint8_t *         p     = (uint8_t *)data;
    static const char hex[] = "0123456789ABCDEF";
    for (int i = 0; i < lens && idx + 4 < mlen; i++) {
        buff[idx++] = hex[p[i] >> 4];
        buff[idx++] = hex[p[i] & 0x0F];
        buff[idx++] = ' ';
    }
    buff[idx++] = '\0';
    output(buff, idx);
    Coroutine.Free(buff, __FILE__, __LINE__);
    return idx;
}

NPLOG_IMPORT(_print, _PrintArray);
