/**
 * @file     nplog.h
 * @brief    日志模块
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
#ifndef __npLog_H__
#define __npLog_H__

/**
 * @brief    日志信息
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-21
 */
typedef struct _NPLOG_Info
{
    int                 level;   // 日志等级
    const char *        name;    // 名称
    struct _NPLOG_Info *next;
} NPLOG_Info;

/**
 * @brief    日志接口
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-21
 */
typedef struct
{
    int (*Print)(const NPLOG_Info *info, int level, const char *file, int line, const char *, ...);
    int (*PrintArray)(const NPLOG_Info *info, int level, const char *file, int line, const void *data, int lens);
} NPLOG_Inter;

/**
 * @brief    日志接口
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-21
 */
typedef struct
{
    int (*Print)(const NPLOG_Info *info, int level, const char *file, int line, const char *, ...);
    int (*PrintArray)(const NPLOG_Info *info, int level, const char *file, int line, const void *data, int lens);
    NPLOG_Info *infos;   // 日志信息列表
} NPLOG;

/**
 * @brief    日志定义
 * @param    tag            日志标签 字符串（不能重复）
 * @param    level          日志等级
 * @note     CXSMethod.Config.h TAG_MAIN "Main"
 * @note     NPLOG_DEFINE(TAG_MAIN, 2)
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-21
 */
#define NPLOG_DEFINE(TAG, Level)                               \
    static NPLOG_Info *CM_S_S(_##TAG, _nplog_info) = nullptr;  \
    CM_INIT_FUNCTION                                           \
    {                                                          \
        static NPLOG_Info info;                                \
        info.name                   = #TAG;                    \
        info.level                  = Level;                   \
        info.next                   = NPLOG_GetInter()->infos; \
        NPLOG_GetInter()->infos     = &info;                   \
        CM_S_S(_##TAG, _nplog_info) = &info;                   \
        return;                                                \
    }

/**
 * @brief    设置日志等级
 * @param    tag            日志标签
 * @param    level          日志等级
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-21
 */
#define NPLOG_SetLevel(TAG, Level)                        \
    do {                                                  \
        NPLOG_Info *p = NPLOG_GetInter()->infos;          \
        while (p != nullptr && strcmp(p->name, TAG) != 0) \
            p = p->next;                                  \
        if (p != nullptr)                                 \
            p->level = Level;                             \
    } while (false)

/**
 * @brief    日志导入
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-07-29
 */
#define NPLOG_IMPORT(Print_func, PrintArray_func)                \
    static NPLOG nplog = {Print_func, PrintArray_func, nullptr}; \
    void         NPLOG_SetInter(const NPLOG_Inter *log)          \
    {                                                            \
        nplog.Print      = log->Print;                           \
        nplog.PrintArray = log->PrintArray;                      \
        return;                                                  \
    }                                                            \
    NPLOG *NPLOG_GetInter(void)                                  \
    {                                                            \
        return &nplog;                                           \
    }


#define NPLOG_LEVEL_DEBUG   0   // 程序调试bug时使用
#define NPLOG_LEVEL_INFO    1   // 程序正常运行时使用
#define NPLOG_LEVEL_WARNING 2   // 程序未按预期运行时使用，但并不是错误，如:用户登录密码错误
#define NPLOG_LEVEL_ERROR   3   // 程序出错误时使用，如:IO操作失败
#define NPLOG_LEVEL_MAX     3

#define PRI_U32      "%u"
#define PRI_U32_N(n) "%" #n "u"
#define PRI_S32      "%d"
#define PRI_S32_N(n) "%" #n "d"

#define PRI_U64      "%llu"
#define PRI_U64_N(n) "%" #n "llu"
#define PRI_S64      "%lld"
#define PRI_S64_N(n) "%" #n "lld"

#define PRI_f32      "%f"
#define PRI_f32_N(n) "%" #n "f"
#define PRI_f64      "%f"
#define PRI_f64_N(n) "%" #n "f"

#define PRI_STR      "%s"
#define PRI_STR_N(n) "%" #n "s"

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_PRINTF_Array(level, data, len) NPLOG_GetInter()->PrintArray(nullptr, level, __FILE__, __LINE__, data, len)
#define LOG_DEBUG(format, ...)             NPLOG_GetInter()->Print(nullptr, NPLOG_LEVEL_DEBUG, __FILE__, __LINE__, format "\n", ##__VA_ARGS__)
#define LOG_PRINTF(format, ...)            NPLOG_GetInter()->Print(nullptr, NPLOG_LEVEL_INFO, __FILE__, __LINE__, format "\n", ##__VA_ARGS__)
#define LOG_WARNING(format, ...)           NPLOG_GetInter()->Print(nullptr, NPLOG_LEVEL_WARNING, __FILE__, __LINE__, format "\n", ##__VA_ARGS__)
#define LOG_ERROR(format, ...)             NPLOG_GetInter()->Print(nullptr, NPLOG_LEVEL_ERROR, __FILE__, __LINE__, format "\n", ##__VA_ARGS__)

#define LOG_TAG_Array(TAG, level, data, len) NPLOG_GetInter()->PrintArray(CM_S_S(_##TAG, _nplog_info), level, __FILE__, __LINE__, data, len)
#define LOG_TAG_DBUG(TAG, format, ...)       NPLOG_GetInter()->Print(CM_S_S(_##TAG, _nplog_info), NPLOG_LEVEL_DEBUG, __FILE__, __LINE__, format "\n", ##__VA_ARGS__)
#define LOG_TAG_INFO(TAG, format, ...)       NPLOG_GetInter()->Print(CM_S_S(_##TAG, _nplog_info), NPLOG_LEVEL_INFO, __FILE__, __LINE__, format "\n", ##__VA_ARGS__)
#define LOG_TAG_WARN(TAG, format, ...)       NPLOG_GetInter()->Print(CM_S_S(_##TAG, _nplog_info), NPLOG_LEVEL_WARNING, __FILE__, __LINE__, format "\n", ##__VA_ARGS__)
#define LOG_TAG_FAIL(TAG, format, ...)       NPLOG_GetInter()->Print(CM_S_S(_##TAG, _nplog_info), NPLOG_LEVEL_ERROR, __FILE__, __LINE__, format "\n", ##__VA_ARGS__)

#define LOG_TAG_GET_LEVEL(TAG) (CM_S_S(_##TAG, _nplog_info)->level)   // 获取日志等级

/**
 * @brief    设置接口
 * @param    log
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-10-18
 */
void NPLOG_SetInter(const NPLOG_Inter *log);

/**
 * @brief    获取接口
 * @return   const NPLOG*
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-10-18
 */
NPLOG *NPLOG_GetInter(void);
#ifdef __cplusplus
}
#endif
#endif
/*
    1. 导入日志模块
    NPLOG_IMPORT(Print,PrintArray);

    2. 使用
    NPLOG_DEFINE(main, NPLOG_LEVEL_DEBUG);
    #undef LOG_PRINTF_Array
    #undef LOG_DEBUG
    #undef LOG_PRINTF
    #undef LOG_WARNING
    #undef LOG_ERROR
    #define LOG_PRINTF_Array(level, data, len) LOG_TAG_Array(main, level, data, len)
    #define LOG_DEBUG(format, ...)             LOG_TAG_DBUG(main, format, ##__VA_ARGS__)
    #define LOG_PRINTF(format, ...)            LOG_TAG_INFO(main, format, ##__VA_ARGS__)
    #define LOG_WARNING(format, ...)           LOG_TAG_WARN(main, format, ##__VA_ARGS__)
    #define LOG_ERROR(format, ...)             LOG_TAG_FAIL(main, format, ##__VA_ARGS__)
*/
