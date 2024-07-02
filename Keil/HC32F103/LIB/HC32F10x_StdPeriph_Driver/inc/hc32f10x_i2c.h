#ifndef __HC32F10x_I2C_H
#define __HC32F10x_I2C_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "hc32f10x.h"



typedef struct
{
  uint32_t I2C_ClockSpeed;          /*!< Specifies the clock frequency.
                                         This parameter must be set to a value lower than 400kHz */

  uint16_t I2C_Mode;                /*!< Specifies the I2C mode.
                                         This parameter can be a value of @ref I2C_mode */

  uint16_t I2C_DutyCycle;           /*!< Specifies the I2C fast mode duty cycle.
                                         This parameter can be a value of @ref I2C_duty_cycle_in_fast_mode */

  uint16_t I2C_OwnAddress1;         /*!< Specifies the first device own address.
                                         This parameter can be a 7-bit or 10-bit address. */

  uint16_t I2C_Ack;                 /*!< Enables or disables the acknowledgement.
                                         This parameter can be a value of @ref I2C_acknowledgement */

  uint16_t I2C_AcknowledgedAddress; /*!< Specifies if 7-bit or 10-bit address is acknowledged.
                                         This parameter can be a value of @ref I2C_acknowledged_address */
}I2C_InitTypeDef;

/*************************************************************************
Register Address 
*************************************************************************/
#define I2C1												0x40005400
#define I2C2												0x40005800

#define I2C_CR1(I2Cn)								(*(volatile uint32_t *)(uint32_t)(I2Cn + 0x00))
#define I2C_CR2(I2Cn)								(*(volatile uint32_t *)(uint32_t)(I2Cn + 0x04))
#define I2C_OAR1(I2Cn)							(*(volatile uint32_t *)(uint32_t)(I2Cn + 0x08))
#define I2C_OAR2(I2Cn)							(*(volatile uint32_t *)(uint32_t)(I2Cn + 0x0C))
#define I2C_DR(I2Cn)								(*(volatile uint32_t *)(uint32_t)(I2Cn + 0x10))
#define I2C_SR1(I2Cn)								(*(volatile uint32_t *)(uint32_t)(I2Cn + 0x14))
#define I2C_SR2(I2Cn)								(*(volatile uint32_t *)(uint32_t)(I2Cn + 0x18))
#define I2C_CCR(I2Cn)								(*(volatile uint32_t *)(uint32_t)(I2Cn + 0x1C))
#define I2C_TRISE(I2Cn)							(*(volatile uint32_t *)(uint32_t)(I2Cn + 0x20))


/** @defgroup I2C_mode 
  * @{
  */

#define I2C_Mode_I2C                    ((uint16_t)0x0000)
#define I2C_Mode_SMBusDevice            ((uint16_t)0x0002)  
#define I2C_Mode_SMBusHost              ((uint16_t)0x000A)


/** @defgroup I2C_duty_cycle_in_fast_mode 
  * @{
  */

#define I2C_DutyCycle_16_9              ((uint16_t)0x4000) /*!< I2C fast mode Tlow/Thigh = 16/9 */
#define I2C_DutyCycle_2                 ((uint16_t)0xBFFF) /*!< I2C fast mode Tlow/Thigh = 2 */


/** @defgroup I2C_acknowledgement
  * @{
  */

#define I2C_Ack_Enable                  ((uint16_t)0x0400)
#define I2C_Ack_Disable                 ((uint16_t)0x0000)


/** @defgroup I2C_transfer_direction 
  * @{
  */

#define  I2C_Direction_Transmitter      ((uint8_t)0x00)
#define  I2C_Direction_Receiver         ((uint8_t)0x01)


/** @defgroup I2C_acknowledged_address 
  * @{
  */

#define I2C_AcknowledgedAddress_7bit    ((uint16_t)0x4000)
#define I2C_AcknowledgedAddress_10bit   ((uint16_t)0xC000)

/** @defgroup I2C_registers 
  * @{
  */

#define I2C_Register_CR1                ((uint8_t)0x00)
#define I2C_Register_CR2                ((uint8_t)0x04)
#define I2C_Register_OAR1               ((uint8_t)0x08)
#define I2C_Register_OAR2               ((uint8_t)0x0C)
#define I2C_Register_DR                 ((uint8_t)0x10)
#define I2C_Register_SR1                ((uint8_t)0x14)
#define I2C_Register_SR2                ((uint8_t)0x18)
#define I2C_Register_CCR                ((uint8_t)0x1C)
#define I2C_Register_TRISE              ((uint8_t)0x20)


/** @defgroup I2C_SMBus_alert_pin_level 
  * @{
  */

#define I2C_SMBusAlert_Low              ((uint16_t)0x2000)
#define I2C_SMBusAlert_High             ((uint16_t)0xDFFF)


/** @defgroup I2C_PEC_position 
  * @{
  */

#define I2C_PECPosition_Next            ((uint16_t)0x0800)
#define I2C_PECPosition_Current         ((uint16_t)0xF7FF)


/** @defgroup I2C_NCAK_position 
  * @{
  */

#define I2C_NACKPosition_Next           ((uint16_t)0x0800)
#define I2C_NACKPosition_Current        ((uint16_t)0xF7FF)


/** @defgroup I2C_interrupts_definition 
  * @{
  */

#define I2C_IT_BUF                      ((uint16_t)0x0400)
#define I2C_IT_EVT                      ((uint16_t)0x0200)
#define I2C_IT_ERR                      ((uint16_t)0x0100)


/** @defgroup I2C_interrupts_definition 
  * @{
  */

#define I2C_IT_SMBALERT                 ((uint32_t)0x01008000)
#define I2C_IT_TIMEOUT                  ((uint32_t)0x01004000)
#define I2C_IT_PECERR                   ((uint32_t)0x01001000)
#define I2C_IT_OVR                      ((uint32_t)0x01000800)
#define I2C_IT_AF                       ((uint32_t)0x01000400)
#define I2C_IT_ARLO                     ((uint32_t)0x01000200)
#define I2C_IT_BERR                     ((uint32_t)0x01000100)
#define I2C_IT_TXE                      ((uint32_t)0x06000080)
#define I2C_IT_RXNE                     ((uint32_t)0x06000040)
#define I2C_IT_STOPF                    ((uint32_t)0x02000010)
#define I2C_IT_ADD10                    ((uint32_t)0x02000008)
#define I2C_IT_BTF                      ((uint32_t)0x02000004)
#define I2C_IT_ADDR                     ((uint32_t)0x02000002)
#define I2C_IT_SB                       ((uint32_t)0x02000001)


/** 
  * @brief  SR2 register flags  
  */

#define I2C_FLAG_DUALF                  ((uint32_t)0x00800000)
#define I2C_FLAG_SMBHOST                ((uint32_t)0x00400000)
#define I2C_FLAG_SMBDEFAULT             ((uint32_t)0x00200000)
#define I2C_FLAG_GENCALL                ((uint32_t)0x00100000)
#define I2C_FLAG_TRA                    ((uint32_t)0x00040000)
#define I2C_FLAG_BUSY                   ((uint32_t)0x00020000)
#define I2C_FLAG_MSL                    ((uint32_t)0x00010000)

/** 
  * @brief  SR1 register flags  
  */

#define I2C_FLAG_SMBALERT               ((uint32_t)0x10008000)
#define I2C_FLAG_TIMEOUT                ((uint32_t)0x10004000)
#define I2C_FLAG_PECERR                 ((uint32_t)0x10001000)
#define I2C_FLAG_OVR                    ((uint32_t)0x10000800)
#define I2C_FLAG_AF                     ((uint32_t)0x10000400)
#define I2C_FLAG_ARLO                   ((uint32_t)0x10000200)
#define I2C_FLAG_BERR                   ((uint32_t)0x10000100)
#define I2C_FLAG_TXE                    ((uint32_t)0x10000080)
#define I2C_FLAG_RXNE                   ((uint32_t)0x10000040)
#define I2C_FLAG_STOPF                  ((uint32_t)0x10000010)
#define I2C_FLAG_ADD10                  ((uint32_t)0x10000008)
#define I2C_FLAG_BTF                    ((uint32_t)0x10000004)
#define I2C_FLAG_ADDR                   ((uint32_t)0x10000002)
#define I2C_FLAG_SB                     ((uint32_t)0x10000001)

/* --EV5 */
#define  I2C_EVENT_MASTER_MODE_SELECT                      ((uint32_t)0x00030001)  /* BUSY, MSL and SB flag */

/* --EV6 */
#define  I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED        ((uint32_t)0x00070082)  /* BUSY, MSL, ADDR, TXE and TRA flags */
#define  I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED           ((uint32_t)0x00030002)  /* BUSY, MSL and ADDR flags */
/* --EV9 */
#define  I2C_EVENT_MASTER_MODE_ADDRESS10                   ((uint32_t)0x00030008)  /* BUSY, MSL and ADD10 flags */

/* Master RECEIVER mode -----------------------------*/ 
/* --EV7 */
#define  I2C_EVENT_MASTER_BYTE_RECEIVED                    ((uint32_t)0x00030040)  /* BUSY, MSL and RXNE flags */

/* Master TRANSMITTER mode --------------------------*/
/* --EV8 */
#define I2C_EVENT_MASTER_BYTE_TRANSMITTING                 ((uint32_t)0x00070080) /* TRA, BUSY, MSL, TXE flags */
/* --EV8_2 */
#define  I2C_EVENT_MASTER_BYTE_TRANSMITTED                 ((uint32_t)0x00070084)  /* TRA, BUSY, MSL, TXE and BTF flags */

/* --EV1  (all the events below are variants of EV1) */   
/* 1) Case of One Single Address managed by the slave */
#define  I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED          ((uint32_t)0x00020002) /* BUSY and ADDR flags */
#define  I2C_EVENT_SLAVE_TRANSMITTER_ADDRESS_MATCHED       ((uint32_t)0x00060082) /* TRA, BUSY, TXE and ADDR flags */

/* 2) Case of Dual address managed by the slave */
#define  I2C_EVENT_SLAVE_RECEIVER_SECONDADDRESS_MATCHED    ((uint32_t)0x00820000)  /* DUALF and BUSY flags */
#define  I2C_EVENT_SLAVE_TRANSMITTER_SECONDADDRESS_MATCHED ((uint32_t)0x00860080)  /* DUALF, TRA, BUSY and TXE flags */

/* 3) Case of General Call enabled for the slave */
#define  I2C_EVENT_SLAVE_GENERALCALLADDRESS_MATCHED        ((uint32_t)0x00120000)  /* GENCALL and BUSY flags */

/* Slave RECEIVER mode --------------------------*/ 
/* --EV2 */
#define  I2C_EVENT_SLAVE_BYTE_RECEIVED                     ((uint32_t)0x00020040)  /* BUSY and RXNE flags */
/* --EV4  */
#define  I2C_EVENT_SLAVE_STOP_DETECTED                     ((uint32_t)0x00000010)  /* STOPF flag */

/* Slave TRANSMITTER mode -----------------------*/
/* --EV3 */
#define  I2C_EVENT_SLAVE_BYTE_TRANSMITTED                  ((uint32_t)0x00060084)  /* TRA, BUSY, TXE and BTF flags */
#define  I2C_EVENT_SLAVE_BYTE_TRANSMITTING                 ((uint32_t)0x00060080)  /* TRA, BUSY and TXE flags */
/* --EV3_2 */
#define  I2C_EVENT_SLAVE_ACK_FAILURE                       ((uint32_t)0x00000400)  /* AF flag */


void I2C_DeInit(uint32_t I2Cx);
void I2C_Init(uint32_t I2Cx, I2C_InitTypeDef* I2C_InitStruct);
void I2C_StructInit(I2C_InitTypeDef* I2C_InitStruct);
void I2C_Cmd(uint32_t I2Cx, FunctionalState NewState);
void I2C_DMACmd(uint32_t I2Cx, FunctionalState NewState);
void I2C_DMALastTransferCmd(uint32_t I2Cx, FunctionalState NewState);
void I2C_GenerateSTART(uint32_t I2Cx, FunctionalState NewState);
void I2C_GenerateSTOP(uint32_t I2Cx, FunctionalState NewState);
void I2C_AcknowledgeConfig(uint32_t I2Cx, FunctionalState NewState);
void I2C_OwnAddress2Config(uint32_t I2Cx, uint8_t Address);
void I2C_DualAddressCmd(uint32_t I2Cx, FunctionalState NewState);
void I2C_GeneralCallCmd(uint32_t I2Cx, FunctionalState NewState);
void I2C_ITConfig(uint32_t I2Cx, uint16_t I2C_IT, FunctionalState NewState);
void I2C_SendData(uint32_t I2Cx, uint8_t Data);
uint8_t I2C_ReceiveData(uint32_t I2Cx);
void I2C_Send7bitAddress(uint32_t I2Cx, uint8_t Address, uint8_t I2C_Direction);
uint16_t I2C_ReadRegister(uint32_t I2Cx, uint8_t I2C_Register);
void I2C_SoftwareResetCmd(uint32_t I2Cx, FunctionalState NewState);
void I2C_NACKPositionConfig(uint32_t I2Cx, uint16_t I2C_NACKPosition);
void I2C_SMBusAlertConfig(uint32_t I2Cx, uint16_t I2C_SMBusAlert);
void I2C_TransmitPEC(uint32_t I2Cx, FunctionalState NewState);
void I2C_PECPositionConfig(uint32_t I2Cx, uint16_t I2C_PECPosition);
void I2C_CalculatePEC(uint32_t I2Cx, FunctionalState NewState);
uint8_t I2C_GetPEC(uint32_t I2Cx);
void I2C_ARPCmd(uint32_t I2Cx, FunctionalState NewState);
void I2C_StretchClockCmd(uint32_t I2Cx, FunctionalState NewState);
void I2C_FastModeDutyCycleConfig(uint32_t I2Cx, uint16_t I2C_DutyCycle);

ErrorStatus I2C_CheckEvent(uint32_t I2Cx, uint32_t I2C_EVENT);
uint32_t I2C_GetLastEvent(uint32_t I2Cx);
FlagStatus I2C_GetFlagStatus(uint32_t I2Cx, uint32_t I2C_FLAG);
void I2C_ClearFlag(uint32_t I2Cx, uint32_t I2C_FLAG);
ITStatus I2C_GetITStatus(uint32_t I2Cx, uint32_t I2C_IT);
void I2C_ClearITPendingBit(uint32_t I2Cx, uint32_t I2C_IT);

#ifdef __cplusplus
}
#endif

#endif 

/*************END OF FILE****/
