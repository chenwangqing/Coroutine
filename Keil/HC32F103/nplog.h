
/*
 模块名： npLog 日志接口
 版本：v0.1
 创建日期：2020年4月28日
 创建人：陈祥树
 修改日期        修改人	版本	修改内容
 2020年6月30日	陈祥树	0.1		添加LOG等级标准
 注意：
 */
#ifndef __npLog_H__
#define __npLog_H__
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

typedef struct
{
    int (*Print)(int level, const char *file, int line, const char *, ...);
    void (*PrintArray)(int level, const char *file, int line, const void *data, int lens);
} NPLOG;

#define NPLOG_LEVEL_DEBUG   0   // 程序调试bug时使用
#define NPLOG_LEVEL_INFO    1   // 程序正常运行时使用
#define NPLOG_LEVEL_WARNING 2   // 程序未按预期运行时使用，但并不是错误，如:用户登录密码错误
#define NPLOG_LEVEL_ERROR   3   // 程序出错误时使用，如:IO操作失败
#define NPLOG_LEVEL_MAX     3

#ifdef __cplusplus
extern "C" {
#endif
extern NPLOG nplog;

#define LOG_PRINTF_Array(level, data, len) nplog.PrintArray(level, __FILE__, __LINE__, data, len)
#define LOG_DEBUG(format, ...)             nplog.Print(NPLOG_LEVEL_DEBUG, __FILE__, __LINE__, format "\n", ##__VA_ARGS__)
#define LOG_PRINTF(format, ...)            nplog.Print(NPLOG_LEVEL_INFO, __FILE__, __LINE__, format "\n", ##__VA_ARGS__)
#define LOG_WARNING(format, ...)           nplog.Print(NPLOG_LEVEL_WARNING, __FILE__, __LINE__, format "\n", ##__VA_ARGS__)
#define LOG_ERROR(format, ...)             nplog.Print(NPLOG_LEVEL_ERROR, __FILE__, __LINE__, format "\n", ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif
#endif
