#ifndef __HC32F10x_USART_H
#define __HC32F10x_USART_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "hc32f10x.h"


  
typedef struct
{
  uint32_t USART_BaudRate;            /*!< This member configures the USART communication baud rate.
                                           The baud rate is computed using the following formula:
                                            - IntegerDivider = ((PCLKx) / (16 * (USART_InitStruct->USART_BaudRate)))
                                            - FractionalDivider = ((IntegerDivider - ((u32) IntegerDivider)) * 16) + 0.5 */

  uint16_t USART_WordLength;          /*!< Specifies the number of data bits transmitted or received in a frame.
                                           This parameter can be a value of @ref USART_Word_Length */

  uint16_t USART_StopBits;            /*!< Specifies the number of stop bits transmitted.
                                           This parameter can be a value of @ref USART_Stop_Bits */

  uint16_t USART_Parity;              /*!< Specifies the parity mode.
                                           This parameter can be a value of @ref USART_Parity
                                           @note When parity is enabled, the computed parity is inserted
                                                 at the MSB position of the transmitted data (9th bit when
                                                 the word length is set to 9 data bits; 8th bit when the
                                                 word length is set to 8 data bits). */
 
  uint16_t USART_Mode;                /*!< Specifies wether the Receive or Transmit mode is enabled or disabled.
                                           This parameter can be a value of @ref USART_Mode */

  uint16_t USART_HardwareFlowControl; /*!< Specifies wether the hardware flow control mode is enabled
                                           or disabled.
                                           This parameter can be a value of @ref USART_Hardware_Flow_Control */
} USART_InitTypeDef;

/** 
  * @brief  USART Clock Init Structure definition  
  */ 
  
typedef struct
{

  uint16_t USART_Clock;   /*!< Specifies whether the USART clock is enabled or disabled.
                               This parameter can be a value of @ref USART_Clock */

  uint16_t USART_CPOL;    /*!< Specifies the steady state value of the serial clock.
                               This parameter can be a value of @ref USART_Clock_Polarity */

  uint16_t USART_CPHA;    /*!< Specifies the clock transition on which the bit capture is made.
                               This parameter can be a value of @ref USART_Clock_Phase */

  uint16_t USART_LastBit; /*!< Specifies whether the clock pulse corresponding to the last transmitted
                               data bit (MSB) has to be output on the SCLK pin in synchronous mode.
                               This parameter can be a value of @ref USART_Last_Bit */
} USART_ClockInitTypeDef;


/*************************************************************************
Register Address 
*************************************************************************/
#define USART1												0x40013800
#define USART2												0x40004400
#define USART3												0x40004800

#define USART_SR(USARTn)								(*(volatile uint32_t *)(uint32_t)(USARTn + 0x00))
#define USART_DR(USARTn)								(*(volatile uint32_t *)(uint32_t)(USARTn + 0x04))
#define USART_BRR(USARTn)								(*(volatile uint32_t *)(uint32_t)(USARTn + 0x08))
#define USART_CR1(USARTn)								(*(volatile uint32_t *)(uint32_t)(USARTn + 0x0C))
#define USART_CR2(USARTn)								(*(volatile uint32_t *)(uint32_t)(USARTn + 0x10))
#define USART_CR3(USARTn)								(*(volatile uint32_t *)(uint32_t)(USARTn + 0x14))
#define USART_GTPR(USARTn)							(*(volatile uint32_t *)(uint32_t)(USARTn + 0x18))


#define USART_WordLength_8b                  ((uint16_t)0x0000)
#define USART_WordLength_9b                  ((uint16_t)0x1000)
                                    
  
#define USART_StopBits_1                     ((uint16_t)0x0000)
#define USART_StopBits_0_5                   ((uint16_t)0x1000)
#define USART_StopBits_2                     ((uint16_t)0x2000)
#define USART_StopBits_1_5                   ((uint16_t)0x3000)

  
#define USART_Parity_No                      ((uint16_t)0x0000)
#define USART_Parity_Even                    ((uint16_t)0x0400)
#define USART_Parity_Odd                     ((uint16_t)0x0600) 

  
#define USART_Mode_Rx                        ((uint16_t)0x0004)
#define USART_Mode_Tx                        ((uint16_t)0x0008)

#define USART_HardwareFlowControl_None       ((uint16_t)0x0000)
#define USART_HardwareFlowControl_RTS        ((uint16_t)0x0100)
#define USART_HardwareFlowControl_CTS        ((uint16_t)0x0200)
#define USART_HardwareFlowControl_RTS_CTS    ((uint16_t)0x0300)

#define USART_Clock_Disable                  ((uint16_t)0x0000)
#define USART_Clock_Enable                   ((uint16_t)0x0800)

  
#define USART_CPOL_Low                       ((uint16_t)0x0000)
#define USART_CPOL_High                      ((uint16_t)0x0400)


#define USART_CPHA_1Edge                     ((uint16_t)0x0000)
#define USART_CPHA_2Edge                     ((uint16_t)0x0200)


#define USART_LastBit_Disable                ((uint16_t)0x0000)
#define USART_LastBit_Enable                 ((uint16_t)0x0100)

  
#define USART_IT_PE                          ((uint16_t)0x0028)
#define USART_IT_TXE                         ((uint16_t)0x0727)
#define USART_IT_TC                          ((uint16_t)0x0626)
#define USART_IT_RXNE                        ((uint16_t)0x0525)
#define USART_IT_IDLE                        ((uint16_t)0x0424)
#define USART_IT_LBD                         ((uint16_t)0x0846)
#define USART_IT_CTS                         ((uint16_t)0x096A)
#define USART_IT_ERR                         ((uint16_t)0x0060)
#define USART_IT_ORE                         ((uint16_t)0x0360)
#define USART_IT_NE                          ((uint16_t)0x0260)
#define USART_IT_FE                          ((uint16_t)0x0160)


#define USART_DMAReq_Tx                      ((uint16_t)0x0080)
#define USART_DMAReq_Rx                      ((uint16_t)0x0040)


#define USART_WakeUp_IdleLine                ((uint16_t)0x0000)
#define USART_WakeUp_AddressMark             ((uint16_t)0x0800)

  
#define USART_LINBreakDetectLength_10b      ((uint16_t)0x0000)
#define USART_LINBreakDetectLength_11b      ((uint16_t)0x0020)


#define USART_IrDAMode_LowPower              ((uint16_t)0x0004)
#define USART_IrDAMode_Normal                ((uint16_t)0x0000)

#define USART_FLAG_CTS                       ((uint16_t)0x0200)
#define USART_FLAG_LBD                       ((uint16_t)0x0100)
#define USART_FLAG_TXE                       ((uint16_t)0x0080)
#define USART_FLAG_TC                        ((uint16_t)0x0040)
#define USART_FLAG_RXNE                      ((uint16_t)0x0020)
#define USART_FLAG_IDLE                      ((uint16_t)0x0010)
#define USART_FLAG_ORE                       ((uint16_t)0x0008)
#define USART_FLAG_NE                        ((uint16_t)0x0004)
#define USART_FLAG_FE                        ((uint16_t)0x0002)
#define USART_FLAG_PE                        ((uint16_t)0x0001)


void USART_DeInit(uint32_t USARTx);
void USART_Init(uint32_t USARTx, USART_InitTypeDef* USART_InitStruct);
void USART_StructInit(USART_InitTypeDef* USART_InitStruct);
void USART_ClockInit(uint32_t USARTx, USART_ClockInitTypeDef* USART_ClockInitStruct);
void USART_ClockStructInit(USART_ClockInitTypeDef* USART_ClockInitStruct);
void USART_Cmd(uint32_t USARTx, FunctionalState NewState);
void USART_ITConfig(uint32_t USARTx, uint16_t USART_IT, FunctionalState NewState);
void USART_DMACmd(uint32_t USARTx, uint16_t USART_DMAReq, FunctionalState NewState);
void USART_SetAddress(uint32_t USARTx, uint8_t USART_Address);
void USART_WakeUpConfig(uint32_t USARTx, uint16_t USART_WakeUp);
void USART_ReceiverWakeUpCmd(uint32_t USARTx, FunctionalState NewState);
void USART_LINBreakDetectLengthConfig(uint32_t USARTx, uint16_t USART_LINBreakDetectLength);
void USART_LINCmd(uint32_t USARTx, FunctionalState NewState);
void USART_SendData(uint32_t USARTx, uint16_t Data);
uint16_t USART_ReceiveData(uint32_t USARTx);
void USART_SendBreak(uint32_t USARTx);
void USART_SetGuardTime(uint32_t USARTx, uint8_t USART_GuardTime);
void USART_SetPrescaler(uint32_t USARTx, uint8_t USART_Prescaler);
void USART_SmartCardCmd(uint32_t USARTx, FunctionalState NewState);
void USART_SmartCardNACKCmd(uint32_t USARTx, FunctionalState NewState);
void USART_HalfDuplexCmd(uint32_t USARTx, FunctionalState NewState);
void USART_OverSampling8Cmd(uint32_t USARTx, FunctionalState NewState);
void USART_OneBitMethodCmd(uint32_t USARTx, FunctionalState NewState);
void USART_IrDAConfig(uint32_t USARTx, uint16_t USART_IrDAMode);
void USART_IrDACmd(uint32_t USARTx, FunctionalState NewState);
FlagStatus USART_GetFlagStatus(uint32_t USARTx, uint16_t USART_FLAG);
void USART_ClearFlag(uint32_t USARTx, uint16_t USART_FLAG);
ITStatus USART_GetITStatus(uint32_t USARTx, uint16_t USART_IT);
void USART_ClearITPendingBit(uint32_t USARTx, uint16_t USART_IT);

#ifdef __cplusplus
}
#endif

#endif 

/*********************END OF FILE****/
