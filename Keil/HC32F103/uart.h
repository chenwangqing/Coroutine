
#if !defined(__UART_HXX__)
#define __UART_HXX__
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UART_SEND_DMA_EN 0
#define UART_RECV_DMA_EN 0

extern void UART_Init(void);
extern int  UART_Write(size_t fd, const void *data, int len);
extern int  UART_Read(size_t fd, void *data, int len, uint32_t timeout);
extern void UART_DeInit(void);

#ifdef __cplusplus
}
#endif
#endif   // __UART_HXX__
