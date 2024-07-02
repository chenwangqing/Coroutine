/**
 * @file     CXSMethod.Define.h
 * @brief    定义
 * @author   CXS (chenxiangshu@outlook.com)
 * @version  1.0
 * @date     2023-08-11
 *
 * @copyright Copyright (c) 2023  Four-Faith
 *
 * @par 修改日志:
 * <table>
 * <tr><th>日期       <th>版本    <th>作者    <th>说明
 * <tr><td>2023-08-11 <td>1.0     <td>CXS     <td>创建
 * </table>
 */
#if !defined(__CXSMETHOD_DEFINE_H__)
#define __CXSMETHOD_DEFINE_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
#include <vector>
#include <map>
#include <list>
#include <initializer_list>
#include <algorithm>
#include <set>
#include <iterator>
#include <string>

typedef std::vector<uint8_t> Bytes;
typedef std::string          string;
typedef std::vector<string>  strings;
#endif

/**
 * @brief    布尔类型
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-03-24
 */
typedef uint8_t Bool;

#ifndef __cplusplus
#ifndef true
#define true (1 == 1)
#endif
#ifndef false
#define false (1 != 1)
#endif
#ifndef nullptr
#define nullptr NULL
#endif
#endif

// 对齐
#define ALIGN(n, m) ((n) <= 0 ? 0 : ((n) - 1) / (m) + 1)

// 元素大小
#define TAB_SIZE(tab) (sizeof(tab) / sizeof(tab[0]))

// FOR [start,end)
#define FOR(type, val, start, end) for (type val = (start); val < (end); val++)

#ifndef MIN
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#define IN
#define OUT

// 是否越界 (按照数值增长方向)
#define IS_OUTBOUNDS(s, e, c) (((s) <= (e)) ? ((c) < (s) || (c) > (e)) : ((c) < (s) && (c) > (e)))

#ifndef PI
#define PI 3.14159265358979323846264338327950288
#endif

/**
 * @brief    时间戳转时间
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-10
 */
#if (defined(_WIN32) || defined(_WIN64))
#define Localtime(times, nowTime) localtime_s(&nowTime, &times);
#elif defined(_ARMABI)
#define Localtime(times, nowTime) _localtime_r(&times, &nowTime);
#else
#define Localtime(times, nowTime) localtime_r(&times, &nowTime);
#endif

#if (defined(_WIN32) || defined(_WIN64))
#define _EXPORT_API      __declspec(dllexport)
#define _STRUCT_ALIGN(n) __declspec(align(n))
#else
#define _EXPORT_API      __attribute__((visibility("default")))
#define _STRUCT_ALIGN(n) __attribute__((aligned(n)))
#endif

/**
 * @brief    结构体对齐
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-18
 */
#define STRUCT_ALIGN(n) _STRUCT_ALIGN(n)

/**
 * @brief    API导出
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-18
 */
#define EXPORT_API _EXPORT_API

#if 0
#ifndef __cplusplus
#define _TINY_MEMCPY(dest, src, len)                                                                                                          \
    ((len) == sizeof(char) ? (void)(*(char *)(dest) = *(char *)(src)) : (len) == sizeof(short) ? (void)(*(short *)(dest) = *(short *)(src))   \
                                                                    : (len) == sizeof(int)     ? (void)(*(int *)(dest) = *(int *)(src))       \
                                                                    : (len) == sizeof(long)    ? (void)(*(long *)(dest) = *(long *)(src))     \
                                                                    : (len) == sizeof(float)   ? (void)(*(float *)(dest) = *(float *)(src))   \
                                                                    : (len) == sizeof(double)  ? (void)(*(double *)(dest) = *(double *)(src)) \
                                                                                               : (void)memcpy((dest), (src), (len)))

#define _CM_SWAP(a, b)                                               \
    do {                                                             \
        char temp[sizeof(a)];                                        \
        static_assert(sizeof(a) == sizeof(b), "Type inconsistency"); \
        _TINY_MEMCPY((void *)temp, &(a), sizeof(a));                 \
        (a) = (b);                                                   \
        (b) = (a);                                                   \
        _TINY_MEMCPY(&(b), (void *)temp, sizeof(a));                 \
    } while (false)
#else
#define _CM_SWAP(a, b) \
    do {               \
        auto t = a;    \
        a      = b;    \
        b      = t;    \
    } while (false)
#endif
#else
#define _CM_SWAP(a, b)       \
    do {                     \
        decltype(a) t = (a); \
        (a)           = (b); \
        (b)           = t;   \
    } while (false)
#endif

/**
 * @brief    数据交换
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-18
 */
#define CM_SWAP(a, b) _CM_SWAP(a, b)

/**
 * @brief    获取结构体字段偏移
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-18
 */
#define CM_STRUCT_OFFSET(structure, field) ((size_t) & ((structure *)0)->field)

#define __INIT_FUNC_NAME(text1, text2) text1##text2

#define CM_S(V) #V

/**
 * @brief    字符串拼接
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-21
 */
#define CM_S_S(text1, text2) __INIT_FUNC_NAME(text1, text2)

/**
 * @brief    宏变量定义
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-09-04
 */
#define CM_VAR_DEF(Type, name) Type CM_S_S(name, CM_S_S(_, __LINE__))

/**
 * @brief    范围限制
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-21
 */
#define CM_RANGE_LIMIT(val, _min, _max) ((val) < (_min) ? (_min) : ((val) > (_max) ? (_max) : (val)))

/**
 * @brief    从字段获取整个结构体
 * @param    Type           结构体类型
 * @param    field          连接字段
 * @param    link           连接点
 * @note     Node *p = NodeLink_ToType(Node, link, a);
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-18
 */
#define CM_Field_ToType(Type, field, link) ((Type *)((char *)(link) - CM_STRUCT_OFFSET(Type, field)))

#ifdef __cplusplus
#define _CM_CLASS_CONSTRUCTION(mem, Type, Args) new (mem) Type Args
#define _CM_CLASS_DESTRUCT(mem, Type)           mem->~Type()
#else
#define _CM_CLASS_CONSTRUCTION(mem, Type, Args)
#define _CM_CLASS_DESTRUCT(mem, Type)
#endif

/**
 * @brief    类构造(p,Node_t,())
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-23
 */
#define CM_CLASS_CONSTRUCTION _CM_CLASS_CONSTRUCTION

/**
 * @brief    类析构(p,Node_t)
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-23
 */
#define CM_CLASS_DESTRUCT _CM_CLASS_DESTRUCT

/**
 * @brief    空白字段
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-26
 */
#define CM_BLANK_FIELD(Type) Type CM_S_S(__, CM_S_S(Type, __LINE__))

/**
 * @brief    零结构
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-28
 */
#define CM_ZERO(ptr) memset(ptr, 0, sizeof(*(ptr)))

/**
 * @brief    零结构
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-28
 */
#define CM_ZERO_N(ptr, size) memset(ptr, 0, size)

/**
 * @brief    获取随机数
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-29
 */
#define CM_RAND(min, max) (rand() % ((max) - (min)) + (min))

#ifndef __cplusplus
#define decltype(x) __typeof__(x)
#endif

#define CM_TAB_SIZE(x) (sizeof(x) / sizeof((x)[0]))

// ----------------------------------------------------------------------------------------------------
//                                          | 初始化  |
// ----------------------------------------------------------------------------------------------------

#ifdef __cplusplus
#define _CM_INIT_FUNCTION                                                                         \
    static void CM_S_S(__init_func_, __LINE__)(void);                                             \
    struct CM_S_S(CM_S_S(__init_func_, __LINE__), _t_)                                            \
    {                                                                                             \
        CM_S_S(CM_S_S(__init_func_, __LINE__), _t_)                                               \
        (void)                                                                                    \
        {                                                                                         \
            CM_S_S(__init_func_, __LINE__)                                                        \
            ();                                                                                   \
        }                                                                                         \
    };                                                                                            \
    static CM_S_S(CM_S_S(__init_func_, __LINE__), _t_) CM_S_S(CM_S_S(__init_func_, __LINE__), _); \
    static void CM_S_S(__init_func_, __LINE__)(void)
#elif defined(_MSC_VER)
#pragma section(".CRT$XCU", read)
#define INITIALIZER2_(p)                                                                              \
    static void CM_S_S(__init_func_, __LINE__)(void);                                                 \
    static __declspec(allocate(".CRT$XCU")) void (*CM_S_S(CM_S_S(__init_func_, __LINE__), _))(void) = \
        CM_S_S(__init_func_, __LINE__);                                                               \
    __pragma(comment(linker, "/include:" CM_S_S(p, CM_S_S(__init_func_, __LINE__)) "_"));             \
    static void CM_S_S(__init_func_, __LINE__)(void)
#ifdef _WIN64
#define _CM_INIT_FUNCTION INITIALIZER2_("")
#else
#define _CM_INIT_FUNCTION INITIALIZER2_("_")
#endif
#else   // gcc
#define _CM_INIT_FUNCTION                                                          \
    static void CM_S_S(__init_func_, __LINE__)(void) __attribute__((constructor)); \
    static void CM_S_S(__init_func_, __LINE__)(void)
#endif

/**
 * @brief    main之前执行（用于列表生成，接口注册，不进行业务功能）
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-17
 */
#define CM_INIT_FUNCTION _CM_INIT_FUNCTION

// ----------------------------------------------------------------------------------------------------
//                                          | 过时标记  |
// ----------------------------------------------------------------------------------------------------

#if __GNUC__
#define _CM_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define _CM_DEPRECATED __declspec(deprecated)
#else
#define _CM_DEPRECATED
#endif

/**
 * @brief    过时标记
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-17
 */
#define CM_DEPRECATED _CM_DEPRECATED

// ----------------------------------------------------------------------------------------------------
//                                          | 类成员检查  |
// ----------------------------------------------------------------------------------------------------

/**
 * @brief    检查类是否有这个成员
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-18
 */
#define CM_CHECK_CLASS_HAS_MEMBERORFUN(type, member) class_has_check_##member<type>::Value
#define CM_CLASS_HAS_MEMBERORFUN_IMPL(member)                                \
    template<typename T>                                                     \
    struct class_has_check_##member                                          \
    {                                                                        \
        template<typename U>                                                 \
        static auto check(...) -> int;                                       \
        template<typename U>                                                 \
        static auto           check(decltype(&U::##member)) -> void;         \
        constexpr static bool Value = std::is_void_v<decltype(check<T>(0))>; \
    };

/* 示例：
    CLASS_HAS_MEMBERORFUN_IMPL(a);
    int main()
    {
        static_assert(CHECK_CLASS_HAS_MEMBERORFUN(PPNODE, a), "xxx");
*/

// ----------------------------------------------------------------------------------------------------
//                                          | 图标  |
// ----------------------------------------------------------------------------------------------------

#define IOC_FAIL    "⛔ "   // 异常
#define IOC_DELETE  "❌ "   // 删除
#define IOC_ISSUE   "⛳ "   // 发布
#define IOC_TAB     "📌 "   // 标记
#define IOC_AMEND   "🛠 "    // 修改
#define IOC_SUCCEED "✅ "   // 功能完成
#define IOC_STAR    "⭐ "   // 标星
#define IOC_NOTE    "💬 "   // 注释
#define IOC_SAFE    "🔰 "   // 安全的，线程安全
#define IOC_DANGER  "⚡ "   // 危险
#define IOC_DOUBT   "❓ "   // 疑问； 技术问题
#define IOC_ANGER   "💢 "   // 生气；很烂的代码
#define IOC_WARN    "❗ "   // 警告
#define IOC_SAVE    "💾 "   // 保存
#define IOC_WAIT    "⏳ "   // 等待

// ----------------------------------------------------------------------------------------------------
//                                          | 设备类型  |
// ----------------------------------------------------------------------------------------------------

#define OS_WINDOWS  0
#define OS_LINUX    1
#define OS_FREERTOS 2

//#ifndef CFG_OS_MODE
//#if (defined(_WIN32) || defined(_WIN64))
//#define CFG_OS_MODE OS_WINDOWS
//#elif __linux__
//#define CFG_OS_MODE OS_LINUX
//#else
//#error "Please set the device type"
//#endif
//#endif

#define OS_IS_LINUX    (CFG_OS_MODE == OS_LINUX)
#define OS_IS_WINDOWS  (CFG_OS_MODE == OS_WINDOWS)
#define OS_IS_FREERTOS (CFG_OS_MODE == OS_FREERTOS)

#define CPU_IS_32BIT 4 == __SIZEOF_POINTER__
#define CPU_IS_64BIT 8 == __SIZEOF_POINTER__

#endif   // __CM_Define_H__

/* 一些符号说明：
 ⛔: 异常
 ❌: 删除
 ⛳: 发布
 📌📍: 标记
 🛠: 修改
 ⛜: 合并
 🎊: 修正完成
 ✅: 功能完成
 ❗: 未能解决
 ❓: 技术问题
 ⭐: 标星
 💾: 保存
 💡: 新点子
 💢: 很烂的代码
 ⚡: 危险
 🔰: 安全的，线程安全
 💬: 注释
*/
