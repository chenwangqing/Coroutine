#ifndef __HC32F10x_CRC_H
#define __HC32F10x_CRC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "hc32f10x.h"

/*************************************************************************
Register Address 
*************************************************************************/
#define CRC_DR				(*(volatile uint32_t *)(uint32_t)(0x40023000))
#define CRC_IDR				(*(volatile uint32_t *)(uint32_t)(0x40023004))
#define CRC_CR				(*(volatile uint32_t *)(uint32_t)(0x40023008))

void CRC_ResetDR(void);
uint32_t CRC_CalcCRC(uint32_t Data);
uint32_t CRC_CalcBlockCRC(uint32_t pBuffer[], uint32_t BufferLength);
uint32_t CRC_GetCRC(void);
void CRC_SetIDRegister(uint8_t IDValue);
uint8_t CRC_GetIDRegister(void);

#ifdef __cplusplus
}
#endif

#endif 

/*****END OF FILE****/
