/**
 * @file     Coroutine.Platform
 * @brief    平台相关的汇编实现
 * @author   CXS (chenxiangshu@outlook.com)
 * @version  1.0
 * @date     2024-07-03
 *
 * @copyright Copyright (c) 2024  Four-Faith
 *
 * @par 修改日志:
 * <table>
 * <tr><th>日期       <th>版本    <th>作者    <th>说明
 * <tr><td>2024-07-03 <td>1.0     <td>CXS     <td>创建
 * </table>
 */
#include <stdint.h>
#include "Coroutine.h"

/**
 * @brief    支持动态分配运行的线程
 * @return   true           可以在多个线程上运行（非同时运行）
 * @return   false          创建任务后就一直在一个线程上运行
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-07-04
 */
static inline bool coroutine_is_dynamic_run_thread(void)
{
    return true;
}

/**
 * @brief    获取默认栈大小
 * @return   uint32_t
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-07-04
 */
static inline uint32_t coroutine_get_stack_default_size(void)
{
#if defined(_ARMABI)
    return 4 * 1024;
#else
    return 32 * 1024;
#endif
}

/**
 * @brief    栈方向
 * @return   true           向地址高增长
 * @return   false          向地址低减少
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2024-07-04
 */
static inline bool coroutine_get_stack_direction(void)
{
    return false;
}

// clang-format off
#if defined(__ARMCC_VERSION) // Arm Cortex-M
static inline __asm void coroutine_enter_task(void *func, void *arg, int *stack)
{
    MOV R3, R0  // R3 = func
	MOV R0, R1  // R0 = arg
	MSR MSP, R2 // 切换栈
	BX R3       // 执行func
}
#elif defined(WIN32) && defined(_MSC_VER) // Windows x86 MSVC
static inline void coroutine_enter_task(void *func, void *arg, int *stack)
{
    __asm {
        // 1. 获取参数到寄存器
        mov eax, dword ptr[func]
        mov ebx, dword ptr[arg]
        mov ecx, dword ptr[stack]
        // 2. 切换栈
        mov esp, ecx
        // 3. 设置函数参数
        push ebx
        // 4. 调用函数
        call eax
    }
}
#elif defined(__linux__) && defined(__x86_64__) // Linux x86_64
static inline void coroutine_enter_task(void *func, void *arg, int *stack)
{
    __asm__ __volatile__(
        "mov %%rcx, %%rsp\n" // 2. 切换栈
        "mov %%rbx, %%rdi\n" // 3. 设置函数参数
        "call *%%rax\n"      // 4. 调用函数
        :
        : "a"(func),"b"(arg),"c"(stack));
}
#elif defined(__linux__) && defined(__arm__) // Linux arm32
static inline void coroutine_enter_task(void *func, void *arg, int *stack)
{
    #warning "Coroutine.Platform: no test"
    __asm__ __volatile__(
        "MOV R3, R0\n" // R3 = func
        "MOV R0, R1\n" // R0 = arg
        "MOV SP, R2\n" // 切换栈
        "BX R3\n"      // 调用函数
        :
        : "r"(func),"r"(arg),"r"(stack));
}
#elif defined(__linux__) && defined(__aarch64__ ) // Linux arm64
static inline void coroutine_enter_task(void *func, void *arg, int *stack)
{
    #warning "Coroutine.Platform: no test"
    __asm__ __volatile__(
        "MOV X3, X0\n" // R3 = func
        "MOV X0, X1\n" // R0 = arg
        "MOV SP, X2\n" // 切换栈
        "BL X3\n"      // 调用函数
        :
        : "r"(func),"r"(arg),"r"(stack));
}
#else
// TODO: 其他平台的实现
#error "Coroutine.Platform: Unsupported platform!"
#endif
// clang-format on