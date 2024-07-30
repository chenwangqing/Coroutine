/**
 * @file     Print.h
 * @brief    格式化输出
 * @author   CXS (chenxiangshu@outlook.com)
 * @version  1.0
 * @date     2024-07-29
 *
 * @copyright Copyright (c) 2024  Four-Faith
 *
 * @par 修改日志:
 * <table>
 * <tr><th>日期       <th>版本    <th>作者    <th>说明
 * <tr><td>2024-07-29 <td>1.0     <td>CXS     <td>创建
 * </table>
 */
#ifndef __PRINT_H__
#define __PRINT_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

// 支持浮点数输出
#ifndef CFG_PRINT_FLOAT_SUPPORT
#define CFG_PRINT_FLOAT_SUPPORT 1
#endif
// 支持长浮点数输出
#if !defined(CFG_PRINT_LONG_FLOAT_SUPPORT) && CFG_PRINT_FLOAT_SUPPORT
#define CFG_PRINT_LONG_FLOAT_SUPPORT 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __arm__   // arm32 不需要支持长浮点，以减少ROM
#undef CFG_PRINT_LONG_FLOAT_SUPPORT
#define CFG_PRINT_LONG_FLOAT_SUPPORT 0
typedef int ssize_t;
#endif

/*
    %%:                              百分号
    %c:       char                   单个字符
    %p:       void*                  指针
    %[FN]s:   const char*            字符串 %s %10s %*s %-10s %-*s
    %[FN]d:   int                    整数 %d %10d %010d %-10d
    %[FN]u:   unsigned int           无符号整数
    %[FN]o:   unsigned int           以八进制形式输出无符号整数 %o %10o %010o %-10o
    %[FN]x:   unsigned int           以十六进制形式输出无符号整数 %x %10x %010x %-10x
    %[FN]X:   unsigned int           以十六进制形式输出无符号整数（大写） %X %10X %010X %-10X
    %[FN]zu:  size_t                 无符号整数
    %[FN]zx:  size_t                 以十六进制形式输出无符号整数
    %[FN]zd:  ssize_t                有符号整数
    %[FN]ld:  long int               整数
    %[FN]lu:  unsigned long int      无符号整数
    %[FN]lo:  unsigned long int      以八进制形式输出无符号整数
    %[FN]lx:  unsigned long int      以十六进制形式输出无符号整数
    %[FN]lX:  unsigned long int      以十六进制形式输出无符号整数（大写）
    %[FN]lld: long long int          整数
    %[FN]llu: unsigned long long int 无符号整数
    %[FN]llo: unsigned long long int 以八进制形式输出无符号整数
    %[FN]llx: unsigned long long int 以十六进制形式输出无符号整数
    %[FN]llX: unsigned long long int 以十六进制形式输出无符号整数（大写）
    %[FN]f:   double                 浮点数
    %[FN]e:   double                 科学计数法
    %[FN]g:   double                 自动选择 %f 或 %e
    %[FN]lf:  long double            浮点数
    %[FN]le:  long double            科学计数法
    %[FN]lg:  long double            自动选择 %f 或 %e

    F: '-' 左对齐  '+' 有符号正数前加空格  '0' 补零 '*' 宽度不确定
    N：最小宽度
*/

/**
 * @brief    格式化字符串
 * @param    buf            输出缓存
 * @param    size           缓存大小
 * @param    format         格式化字符串
 * @param    ...            参数列表
 * @return   size_t
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-07-29
 */
extern size_t Print(char *buf, size_t size, const char *format, ...);

/**
 * @brief    格式化字符串
 * @param    buf            输出缓存
 * @param    size           缓存大小
 * @param    format         格式化字符串
 * @param    vArgList       参数列表
 * @return   size_t
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-07-29
 */
extern size_t vPrint(char *buf, size_t size, const char *format, va_list vArgList);

#ifdef __cplusplus
}
#endif
#endif
