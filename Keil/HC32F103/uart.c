
#include "uart.h"
#include "app_cfg.h"

/**
 * @brief    FIFO
 * @author   CXS (chenxiangshu@outlook.com)
 * @date     2023-08-10
 */
typedef struct
{
    uint8_t *buff;
    size_t   size;
    size_t   ridx;
    size_t   widx;
    size_t   count;
    uint64_t time;
} CM_FIFO;

typedef struct
{
    uint32_t interface;
    uint32_t ReMap;
    uint32_t PortTx;
    uint16_t PinTx;
    uint32_t PortRx;
    uint16_t PinRx;
    uint32_t BaudRate;
    uint32_t dma_tx;
    uint32_t dma_rx;
    uint8_t *TxBuffer;
    uint16_t TxBufferSize;
    uint8_t *RxBuffer;
    uint16_t RxBufferSize;
    CM_FIFO *fifo;
} UART_ConfigTypeDef;

#if UART_SEND_DMA_EN
static uint8_t UART_Debug_TxBuffer[256];
#else
static uint8_t UART_Debug_TxBuffer[1];
#endif
#if UART_RECV_DMA_EN
static uint8_t UART_Debug_RxBuffer[512 + 64];
#else
static uint8_t UART_Debug_RxBuffer[1];
#endif
static uint8_t UART_Debug_RxFIFOBuffer[0 + 128];
static CM_FIFO UART_Debug_RxFIFO = {UART_Debug_RxFIFOBuffer, sizeof(UART_Debug_RxFIFOBuffer), 0, 0, 0};
#if UART_SEND_DMA_EN
static uint8_t UART_UI_TxBuffer[512 + 64];
#else
static uint8_t UART_UI_TxBuffer[1];
#endif
#if UART_RECV_DMA_EN
static uint8_t UART_UI_RxBuffer[512 + 64];
#else
static uint8_t UART_UI_RxBuffer[1];
#endif
static uint8_t UART_UI_RxFIFOBuffer[0 + 128];
static CM_FIFO UART_UI_RxFIFO = {UART_UI_RxFIFOBuffer, sizeof(UART_UI_RxFIFOBuffer), 0, 0, 0};

extern uint64_t GetMilliseconds(void);

// ----------------------------------------------------------------------------------------------------
//                                          | FIFO  |
// ----------------------------------------------------------------------------------------------------

void CM_FIFO_Init(CM_FIFO *fifo, void *buff, size_t size)
{
    fifo->buff  = (uint8_t *)buff;
    fifo->ridx  = 0;
    fifo->widx  = 0;
    fifo->count = 0;
    return;
}

size_t CM_FIFO_Write(CM_FIFO *fifo, const void *data, size_t size)
{
    size_t i = 0;
    for (; i < size; i++) {
        size_t nidx = (fifo->widx + 1) % fifo->size;
        if (nidx == fifo->ridx) break;   // 缓冲区已满，不能写入数据
        fifo->buff[fifo->widx] = ((uint8_t *)data)[i];
        fifo->widx             = nidx;
    }
    fifo->count += i;
    return i;
}

size_t CM_FIFO_Read(CM_FIFO *fifo, void *data, size_t size)
{
    size_t i = 0;
    for (; i < size; i++) {
        if (fifo->ridx == fifo->widx) break;   // 缓冲区为空，不能读取数据
        ((uint8_t *)data)[i] = fifo->buff[fifo->ridx];
        fifo->ridx           = (fifo->ridx + 1) % fifo->size;
    }
    fifo->count -= i;
    return i;
}

size_t CM_FIFO_GetCount(CM_FIFO *fifo)
{
    return fifo->count;   // fifo->widx < fifo->ridx ? fifo->widx + fifo->size - fifo->ridx : fifo->widx - fifo->ridx;
}

// ----------------------------------------------------------------------------------------------------
//                                          | 串口处理  |
// ----------------------------------------------------------------------------------------------------

static const UART_ConfigTypeDef UART_Config[2] = {
    UART_UI_CONFIG,
    UART_DEBUG_CONFIG,
};

#if UART_RECV_DMA_EN
static void CheckDMA(const UART_ConfigTypeDef *cfg)
{
    // 检查DMA接收
    if (DMA_GetCurrDataCounter(DMA1, cfg->dma_rx) != cfg->RxBufferSize) {
        // 关闭DMA
        DMA_Cmd(DMA1, cfg->dma_rx, DISABLE);
        // 获取串口数据
        size_t dma_rx_count = cfg->RxBufferSize - DMA_GetCurrDataCounter(DMA1, cfg->dma_rx);
        CM_FIFO_Write(cfg->fifo, cfg->RxBuffer, dma_rx_count);
        //重新配置
        DMA_SetCurrDataCounter(DMA1, cfg->dma_rx, cfg->RxBufferSize);
        // 启动DMA接收
        DMA_Cmd(DMA1, cfg->dma_rx, ENABLE);
    }
    return;
}
#endif

static void USART_IRQHandler(const UART_ConfigTypeDef *cfg)
{
#if UART_RECV_DMA_EN
    if (USART_GetITStatus(cfg->interface, USART_IT_IDLE)) {
        // 清除空闲中断
        USART_ReceiveData(cfg->interface);
        CheckDMA(cfg);
    }
#endif
    if (USART_GetITStatus(cfg->interface, USART_IT_RXNE)) {
        uint8_t ch = USART_ReceiveData(cfg->interface);
        CM_FIFO_Write(cfg->fifo, &ch, 1);
        // cfg->fifo->time = GetMilliseconds();
    }
    return;
}

void USART1_IRQHandler(void)
{
    const UART_ConfigTypeDef *cfg = &UART_Config[0];
    USART_IRQHandler(cfg);
    return;
}

void USART2_IRQHandler(void)
{
    const UART_ConfigTypeDef *cfg = &UART_Config[1];
    USART_IRQHandler(cfg);
    return;
}

#if UART_RECV_DMA_EN
static void DMA_Channel_IRQHandler(void)
{
    const UART_ConfigTypeDef *cfg = NULL;
    if (DMA_GetITStatus(DMA1_FLAG_HT5)) {
        DMA_ClearITPendingBit(DMA1_FLAG_HT5);
        cfg = &UART_Config[0];

    } else if (DMA_GetITStatus(DMA1_FLAG_HT6)) {
        DMA_ClearITPendingBit(DMA1_FLAG_HT6);
        cfg = &UART_Config[1];
    }
    if (cfg == NULL) return;
    CheckDMA(cfg);
    return;
}

void DMA1_Channel5_IRQHandler(void)
{
    DMA_Channel_IRQHandler();
}

void DMA1_Channel6_IRQHandler(void)
{
    DMA_Channel_IRQHandler();
}
#endif

static void BSP_UART_Init(const UART_ConfigTypeDef *cfg)
{
    GPIO_InitTypeDef  GPIO_InitStructure  = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    DMA_InitTypeDef   DMA_InitStructure   = {0};
    (void)DMA_InitStructure;
    // 初始化DMA
#if UART_RECV_DMA_EN
    DMA_DeInit(DMA1, cfg->dma_rx);
    DMA_Cmd(DMA1, cfg->dma_rx, DISABLE);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART_DR(cfg->interface);   //& (cfg->interface->DATAR);
    DMA_InitStructure.DMA_MemoryBaseAddr     = (u32)cfg->RxBuffer;
    DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_BufferSize         = cfg->RxBufferSize;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode               = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority           = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(DMA1, cfg->dma_rx, &DMA_InitStructure);
#endif
#if UART_SEND_DMA_EN
    DMA_DeInit(DMA1, cfg->dma_tx);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART_DR(cfg->interface);   //& (cfg->interface->DATAR);
    DMA_InitStructure.DMA_MemoryBaseAddr     = (u32)cfg->TxBuffer;
    DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize         = 0;
    DMA_Init(DMA1, cfg->dma_tx, &DMA_InitStructure);
#endif
    // 初始化IO
    GPIO_InitStructure.GPIO_Pin   = cfg->PinTx;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(cfg->PortTx, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin  = cfg->PinRx;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(cfg->PortRx, &GPIO_InitStructure);
    if (cfg->ReMap) GPIO_PinRemapConfig(cfg->ReMap, ENABLE);
    // 初始化串口
    USART_InitStructure.USART_BaudRate            = cfg->BaudRate;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(cfg->interface, &USART_InitStructure);
#if UART_RECV_DMA_EN
    // 设置空闲中断
    USART_ITConfig(cfg->interface, USART_IT_IDLE, ENABLE);
    // 启动DMA接收
    DMA_Cmd(DMA1, cfg->dma_rx, ENABLE);
#else
    USART_ITConfig(cfg->interface, USART_IT_RXNE, ENABLE);
#endif
    // 使能串口
    USART_Cmd(cfg->interface, ENABLE);
#if UART_RECV_DMA_EN
    USART_DMACmd(cfg->interface, USART_DMAReq_Rx, ENABLE);
#endif
#if UART_SEND_DMA_EN
    USART_DMACmd(cfg->interface, USART_DMAReq_Tx, ENABLE);
#endif
    return;
}

static void BSP_NVIC_Init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure                  = {0};
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    // 启用中断
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_Init(&NVIC_InitStructure);
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_Init(&NVIC_InitStructure);
#if UART_RECV_DMA_EN
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
    NVIC_Init(&NVIC_InitStructure);
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;
    NVIC_Init(&NVIC_InitStructure);
    // 接收一半中断
    DMA_ITConfig(DMA1, UART_Config[0].dma_rx, DMA_IT_HT, ENABLE);
    DMA_ITConfig(DMA1, UART_Config[1].dma_rx, DMA_IT_HT, ENABLE);
#endif
    return;
}

void UART_Init(void)
{
    BSP_UART_Init(&UART_Config[0]);
    BSP_UART_Init(&UART_Config[1]);
    BSP_NVIC_Init();
    return;
}
void UART_DeInit(void)
{
    USART_DeInit(UART_Config[0].interface);
    USART_DeInit(UART_Config[1].interface);
    return;
}

int UART_Write(size_t fd, const void *data, int len)
{
    if (data == NULL || len == 0 || fd > 2)
        return 0;
    const UART_ConfigTypeDef *cfg = &UART_Config[fd];
#if UART_SEND_DMA_EN
    if (len > cfg->TxBufferSize)
        len = cfg->TxBufferSize;
    // 检查是否发送完成
    while (DMA_GetCurrDataCounter(DMA1, cfg->dma_tx))
        ;
    memcpy(cfg->TxBuffer, data, len);
    __disable_irq();
    DMA_Cmd(DMA1, cfg->dma_tx, DISABLE);
    // 配置DMA发送
    DMA_SetCurrDataCounter(DMA1, cfg->dma_tx, len);
    // 启动DMA发送
    DMA_Cmd(DMA1, cfg->dma_tx, ENABLE);
    __enable_irq();
#else
    const uint8_t *p  = (const uint8_t *)data;
    uint64_t       ts = GetMilliseconds();
    for (int i = 0; i < len; i++) {
        USART_SendData(cfg->interface, p[i]);
        while (!USART_GetFlagStatus(cfg->interface, USART_FLAG_TC)) {
            if ((GetMilliseconds() - ts) >= 500)
                return len;
        }
        ts = GetMilliseconds();
    }
#endif
    return len;
}

int UART_Read(size_t fd, void *data, int len, uint32_t timeout)
{
    if (fd > 2) return 0;
    const UART_ConfigTypeDef *cfg = &UART_Config[fd];
    int                       cnt = CM_FIFO_GetCount(cfg->fifo);
    // if (GetMilliseconds() - cfg->fifo->time <= 50 && cnt < 1024) return 0;
    if (!cnt) return 0;
    return CM_FIFO_Read(cfg->fifo, data, len);
}
