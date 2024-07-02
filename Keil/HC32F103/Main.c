
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "uart.h"
#include <stdlib.h>
#include <stdio.h>
#include "hc32f10x.h"
#include "nplog.h"
#include "Coroutine.h"

extern void PrintMemory(void);

static void Task1(void *arg)
{
    while (true) {
        Coroutine.YieldDelay(1000);
        LOG_DEBUG("[%llu][1]hello", Coroutine.GetMillisecond());
        PrintMemory();
    }
}

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    // 时钟初始化
    SystemCoreClockUpdate();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
                               RCC_APB2Periph_GPIOB |
                               RCC_APB2Periph_GPIOC |
                               RCC_APB2Periph_GPIOD |
                               RCC_APB2Periph_GPIOE,
                           ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    // 初始化定时器
    SysTick_Config(SystemCoreClock / 1000);
    NVIC_EnableIRQ(SysTick_IRQn);
    // 初始化串口
    UART_Init();

    extern const Coroutine_Inter *GetInter(void);
    const Coroutine_Inter *       inter = GetInter();
    Coroutine.SetInter(inter);
    // 初始化信号

    // 添加任务
    Coroutine.AddTask(-1, Task1, nullptr, TASK_PRI_NORMAL, "Task1");

    while (true)
        Coroutine.RunTick();
    // return 0;
}
