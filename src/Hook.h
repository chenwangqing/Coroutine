
#if !defined(__HOOK_H__)
#define __HOOK_H__
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief    初始化
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-09-05
 */
extern void Hook_Init(void);

/**
 * @brief    忽略动态库
 * @param    exp            通配符表达式
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-09-05
 */
extern void Hook_Ignore(const char *exp);

/**
 * @brief    准备注册
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-09-05
 */
extern void Hook_ReadyRegister(void);

/**
 * @brief    注册
 * @param    exp            通配符表达式
 * @param    name           函数名称
 * @param    func           执行函数
 * @return   true           注册成功
 * @return   false          
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-09-05
 */
extern bool Hook_Register(const char *exp, const char *name, void *func);

/**
 * @brief    完成
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-09-05
 */
extern void Hook_Finish(void);
#ifdef __cplusplus
}
#endif
#endif   // __HOOK_H__
