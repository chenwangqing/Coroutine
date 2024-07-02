
#ifndef __snprintf_h__
#define __snprintf_h__
#include <stdio.h>
#include <stdarg.h>
#ifdef __cplusplus
extern "C"
{
#endif
    /**
     * @brief    安全的vsnprintf
     * @param    buf
     * @param    maxlen
     * @param    format
     * @param    val
     * @return   size_t         实际写入大小
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2021-12-16
     */
    extern size_t ff_vsnprintf(char* buf, size_t maxlen, const char* format, va_list val);

    /**
     * @brief    安全ff_snprintff
     * @param    buf
     * @param    maxlen
     * @param    format
     * @param    ...
     * @return   size_t
     * @author   CXS (chenxiangshu@outlook.com)
     * @date     2021-12-16
     */
    extern size_t ff_snprintf(char* buf, size_t maxlen, const char* format, ...);
#ifdef __cplusplus
}
#endif
#endif
