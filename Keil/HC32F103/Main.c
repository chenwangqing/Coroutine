
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "uart.h"
#include <stdlib.h>
#include <stdio.h>
#include "hc32f10x.h"

static volatile uint64_t Millisecond_count = 0;

void SysTick_Handler(void)
{
    Millisecond_count++;
    return;
}

uint64_t GetMilliseconds(void)
{
	return 0;
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
    
    return 0;
}
