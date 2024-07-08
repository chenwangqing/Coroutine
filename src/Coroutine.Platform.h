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
#ifdef WIN32
typedef uint32_t jmp_buf[6];
#else
#include <setjmp.h>
#endif


// --------------------------------------------------------------------------------------
//                              |   跳转处理    |
// --------------------------------------------------------------------------------------

#ifdef WIN32
// VC 编译器 longjmp 会析构C++ 对象，使用需要自己实现 setjmp / longjmp

/**
 * @brief    保存跳转环境
 * @param    buf            环境
 * @return   int            跳转返回值
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-19
 */
static int setjmp(jmp_buf buf)
{
    __asm {
        mov eax, dword ptr[buf]
        mov dword ptr[eax + 0], ebx   // 保存 ebx
        mov dword ptr[eax + 4], esi   // 保存 esi
        mov dword ptr[eax + 8], edi   // 保存 edi
        mov ecx, ebp
        add ecx, 8
        mov dword ptr[eax + 12], ecx   // 保存 esp
        mov ecx, dword ptr[ebp]
        mov dword ptr[eax + 16], ecx   // 保存 edp
        mov ecx, dword ptr[ebp + 4]   // 获取 调用 setjmp 下一个地址
        mov dword ptr[eax + 20], ecx   // 保存 setjmp 下一个地址
        mov eax, 0   // 默认返回 0
    }
}

/**
 * @brief    执行跳转
 * @param    buf            环境
 * @param    val            跳转返回值
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2022-08-19
 */
static void longjmp(jmp_buf buf, int val)
{
    __asm {
        mov eax, dword ptr[val]
        mov edi, dword ptr[buf]
        mov esi, dword ptr[edi + 4]   // 保存 esi
        mov esp, dword ptr[edi + 12]   // 保存 esp
        mov ebp, dword ptr[edi + 16]   // 保存 edp
        mov ecx, dword ptr[edi + 20]   // 保存 setjmp 下一个地址
        mov ebx, dword ptr[edi + 0]   // 保存 ebx
        mov edi, dword ptr[edi + 8]   // 保存 edi
        jmp ecx
    }
    return;
}
#endif

// --------------------------------------------------------------------------------------
//                              |   平台参数    |
// --------------------------------------------------------------------------------------

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

// --------------------------------------------------------------------------------------
//                              |   函数建立    |
// --------------------------------------------------------------------------------------

// clang-format off
#if defined(__riscv) && __riscv_xlen == 32 // RISC-V 32
static inline  void coroutine_enter_task(void *func, void *arg, int *stack)
{
    int ret;
    __asm volatile("mv a0, %2\n"
        "mv sp, %3\n"
        "jalr %1"
        : "=r"(ret)
        : "r" (func),"r"(arg),"r"(stack)
    );
    (void)ret;
}
#elif defined(__ARMCC_VERSION) // Arm Cortex-M
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
