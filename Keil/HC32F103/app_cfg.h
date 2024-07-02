
#ifndef __APP_CFG_H__
#define __APP_CFG_H__

#include "hc32f10x.h"
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EN_DEBUG 1   // 调试开关

#define APP_VERSION "1.0.0"   // 应用版本号

#define CUTTER_TIMEOUT 10   // 切刀超时时间，单位：ms

#define EN_IWDG 0   // 使能看门狗

#define REMOTE_FLASH_PARS_START_ADDR 0   // 远程参数起始地址
#define REMOTE_FLASH_FILE_START_ADDR 4096   // 远程升级文件起始地址

// 数据记录存储
#define FLASH_BLOCK_SIZE                (4 * 1024)
#define FLASH_RECORDSTORAGE_BLOCK_COUNT 4
#define FLASH_RECORDSTORAGE_ADDR        (0x8000000 + (64 << 10) - FLASH_RECORDSTORAGE_BLOCK_COUNT * FLASH_BLOCK_SIZE)


#define APP_RUN_ADDR 0x08002000   // 应用运行地址

#define NAME_UART_UI     "1"   // 连接UI的串口名称
#define NAME_UART_PCLINK "0"   // 连接PCLink的串口名称

#ifndef WIN32
// UUID 地址  8 字节
#define MCU_UUID_GET     ((const volatile uint32_t *)0x40022300)
#define MCU_UUID_STORE   ((const volatile uint32_t *)(APP_RUN_ADDR + 0x2000))
#define CHECK_MCU_UUID() (FUNC_MCU_UUID_GET(0) == MCU_UUID_STORE[0] && FUNC_MCU_UUID_GET(1) == MCU_UUID_STORE[1])
static inline uint32_t FUNC_MCU_UUID_GET(int idx)
{
    uint32_t id = MCU_UUID_GET[idx];
    return id ^ 0x32315638 ^ idx;
}
#else
#define CHECK_MCU_UUID() true
#endif

// 串口1
#define UART_UI_TX_PORT GPIOA
#define UART_UI_TX_PIN  GPIO_Pin_9
#define UART_UI_RX_PORT GPIOA
#define UART_UI_RX_PIN  GPIO_Pin_10
#define UART_UI_CONFIG  USART1,                  \
                       0, /* ReMap */            \
                       UART_UI_TX_PORT,          \
                       UART_UI_TX_PIN,           \
                       UART_UI_RX_PORT,          \
                       UART_UI_RX_PIN,           \
                       115200,                   \
                       DMA_Channel4,             \
                       DMA_Channel5,             \
                       UART_UI_TxBuffer,         \
                       sizeof(UART_UI_TxBuffer), \
                       UART_UI_RxBuffer,         \
                       sizeof(UART_UI_RxBuffer), \
                       &UART_UI_RxFIFO

// 串口2
#define UART_DEBUG_TX_PORT GPIOA
#define UART_DEBUG_TX_PIN  GPIO_Pin_2
#define UART_DEBUG_RX_PORT GPIOA
#define UART_DEBUG_RX_PIN  GPIO_Pin_3
#define UART_DEBUG_CONFIG  USART2,                          \
                          0, /* GPIO_Remap_USART2, ReMap */ \
                          UART_DEBUG_TX_PORT,               \
                          UART_DEBUG_TX_PIN,                \
                          UART_DEBUG_RX_PORT,               \
                          UART_DEBUG_RX_PIN,                \
                          115200,                           \
                          DMA_Channel7,                     \
                          DMA_Channel6,                     \
                          UART_Debug_TxBuffer,              \
                          sizeof(UART_Debug_TxBuffer),      \
                          UART_Debug_RxBuffer,              \
                          sizeof(UART_Debug_RxBuffer),      \
                          &UART_Debug_RxFIFO

// 切刀状态 INR6
#define GPIO_CUTTER_STATUS_PORT GPIOA
#define GPIO_CUTTER_STATUS_PIN  GPIO_Pin_1
// 纸张状态 INR4
#define GPIO_PAPER_STATUS_PORT GPIOB
#define GPIO_PAPER_STATUS_PIN  GPIO_Pin_0
// 编码轮中断 INR1
#define GPIO_CODINGWHEEL_PORT     GPIOB
#define GPIO_CODINGWHEEL_PIN      GPIO_Pin_2
#define GPIO_CODINGWHEEL_EXIT_IDX 2
// 切刀 COOL
#define GPIO_CUTTER_PORT GPIOC
#define GPIO_CUTTER_PIN  GPIO_Pin_15
// 滚轮 进纸电机 LW HW
#define GPIO_ROLLER_IN1_PORT GPIOB
#define GPIO_ROLLER_IN1_PIN  GPIO_Pin_12
#define GPIO_ROLLER_IN2_PORT GPIOB
#define GPIO_ROLLER_IN2_PIN  GPIO_Pin_13
// 翻盖开关检测GPIO，低电平有效 INR3
#define GPIO_DOOR_SWITCH_PORT GPIOA
#define GPIO_DOOR_SWITCH_PIN  GPIO_Pin_5
// GPIO控制继电器吸合输出220V，高电平有效 ACEN
#define GPIO_RELAY_220V_PORT GPIOA
#define GPIO_RELAY_220V_PIN  GPIO_Pin_4
// GPIO控制可控硅光耦输出220V，低电平有效 P_SOL
#define GPIO_GALV_220V_PORT GPIOA
#define GPIO_GALV_220V_PIN  GPIO_Pin_6
// 按键
#define GPIO_BUTTONS {GPIOB, GPIO_Pin_15}, /* D0 */ \
                     {GPIOA, GPIO_Pin_8},  /* D1 */ \
                     {GPIOA, GPIO_Pin_15}, /* D2 */ \
                     {GPIOB, GPIO_Pin_3},  /* D3 */ \
                     {GPIOB, GPIO_Pin_4},  /* D4 */ \
                     {GPIOB, GPIO_Pin_5},  /* D5 */ \
                     {GPIOB, GPIO_Pin_6},  /* D6 */ \
                     {GPIOB, GPIO_Pin_7},  /* D7 */ \
                     {GPIOB, GPIO_Pin_8},  /* D8 */ \
                     {GPIOB, GPIO_Pin_9},  /* D9 */

typedef struct
{
    uint32_t Port;
    uint32_t Pin;
} GPIO_Key_t;

// 按键数组
extern const GPIO_Key_t Keys[10];

#ifdef __cplusplus
}
#endif
#endif
