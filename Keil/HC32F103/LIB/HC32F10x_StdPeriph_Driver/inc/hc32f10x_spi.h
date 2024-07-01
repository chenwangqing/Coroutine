#ifndef __HC32F10x_SPI_H
#define __HC32F10x_SPI_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "hc32f10x.h"


typedef struct
{
  uint16_t SPI_Direction;           /*!< Specifies the SPI unidirectional or bidirectional data mode.
                                         This parameter can be a value of @ref SPI_data_direction */

  uint16_t SPI_Mode;                /*!< Specifies the SPI operating mode.
                                         This parameter can be a value of @ref SPI_mode */

  uint16_t SPI_DataSize;            /*!< Specifies the SPI data size.
                                         This parameter can be a value of @ref SPI_data_size */

  uint16_t SPI_CPOL;                /*!< Specifies the serial clock steady state.
                                         This parameter can be a value of @ref SPI_Clock_Polarity */

  uint16_t SPI_CPHA;                /*!< Specifies the clock active edge for the bit capture.
                                         This parameter can be a value of @ref SPI_Clock_Phase */

  uint16_t SPI_NSS;                 /*!< Specifies whether the NSS signal is managed by
                                         hardware (NSS pin) or by software using the SSI bit.
                                         This parameter can be a value of @ref SPI_Slave_Select_management */
 
  uint16_t SPI_BaudRatePrescaler;   /*!< Specifies the Baud Rate prescaler value which will be
                                         used to configure the transmit and receive SCK clock.
                                         This parameter can be a value of @ref SPI_BaudRate_Prescaler.
                                         @note The communication clock is derived from the master
                                               clock. The slave clock does not need to be set. */

  uint16_t SPI_FirstBit;            /*!< Specifies whether data transfers start from MSB or LSB bit.
                                         This parameter can be a value of @ref SPI_MSB_LSB_transmission */

  uint16_t SPI_CRCPolynomial;       /*!< Specifies the polynomial used for the CRC calculation. */
}SPI_InitTypeDef;

/** 
  * @brief  I2S Init structure definition  
  */

typedef struct
{

  uint16_t I2S_Mode;         /*!< Specifies the I2S operating mode.
                                  This parameter can be a value of @ref I2S_Mode */

  uint16_t I2S_Standard;     /*!< Specifies the standard used for the I2S communication.
                                  This parameter can be a value of @ref I2S_Standard */

  uint16_t I2S_DataFormat;   /*!< Specifies the data format for the I2S communication.
                                  This parameter can be a value of @ref I2S_Data_Format */

  uint16_t I2S_MCLKOutput;   /*!< Specifies whether the I2S MCLK output is enabled or not.
                                  This parameter can be a value of @ref I2S_MCLK_Output */

  uint32_t I2S_AudioFreq;    /*!< Specifies the frequency selected for the I2S communication.
                                  This parameter can be a value of @ref I2S_Audio_Frequency */

  uint16_t I2S_CPOL;         /*!< Specifies the idle state of the I2S clock.
                                  This parameter can be a value of @ref I2S_Clock_Polarity */
}I2S_InitTypeDef;

/*************************************************************************
Register Address 
*************************************************************************/
#define SPI1												0x40013000
#define SPI2												0x40003800

#define SPI_CR1(SPIn)								(*(volatile uint32_t *)(uint32_t)(SPIn + 0x00))
#define SPI_CR2(SPIn)								(*(volatile uint32_t *)(uint32_t)(SPIn + 0x04))
#define SPI_SR(SPIn)								(*(volatile uint32_t *)(uint32_t)(SPIn + 0x08))
#define SPI_DR(SPIn)								(*(volatile uint32_t *)(uint32_t)(SPIn + 0x0C))
#define SPI_CRCPR(SPIn)							(*(volatile uint32_t *)(uint32_t)(SPIn + 0x10))
#define SPI_RXCRCR(SPIn)						(*(volatile uint32_t *)(uint32_t)(SPIn + 0x14))
#define SPI_TXCRCR(SPIn)						(*(volatile uint32_t *)(uint32_t)(SPIn + 0x18))
  
#define SPI_Direction_2Lines_FullDuplex ((uint16_t)0x0000)
#define SPI_Direction_2Lines_RxOnly     ((uint16_t)0x0400)
#define SPI_Direction_1Line_Rx          ((uint16_t)0x8000)
#define SPI_Direction_1Line_Tx          ((uint16_t)0xC000)

#define SPI_Mode_Master                 ((uint16_t)0x0104)
#define SPI_Mode_Slave                  ((uint16_t)0x0000)

#define SPI_DataSize_16b                ((uint16_t)0x0800)
#define SPI_DataSize_8b                 ((uint16_t)0x0000)

#define SPI_CPOL_Low                    ((uint16_t)0x0000)
#define SPI_CPOL_High                   ((uint16_t)0x0002)

#define SPI_CPHA_1Edge                  ((uint16_t)0x0000)
#define SPI_CPHA_2Edge                  ((uint16_t)0x0001)

#define SPI_NSS_Soft                    ((uint16_t)0x0200)
#define SPI_NSS_Hard                    ((uint16_t)0x0000)

#define SPI_BaudRatePrescaler_2         ((uint16_t)0x0000)
#define SPI_BaudRatePrescaler_4         ((uint16_t)0x0008)
#define SPI_BaudRatePrescaler_8         ((uint16_t)0x0010)
#define SPI_BaudRatePrescaler_16        ((uint16_t)0x0018)
#define SPI_BaudRatePrescaler_32        ((uint16_t)0x0020)
#define SPI_BaudRatePrescaler_64        ((uint16_t)0x0028)
#define SPI_BaudRatePrescaler_128       ((uint16_t)0x0030)
#define SPI_BaudRatePrescaler_256       ((uint16_t)0x0038)

#define SPI_FirstBit_MSB                ((uint16_t)0x0000)
#define SPI_FirstBit_LSB                ((uint16_t)0x0080)

#define I2S_Mode_SlaveTx                ((uint16_t)0x0000)
#define I2S_Mode_SlaveRx                ((uint16_t)0x0100)
#define I2S_Mode_MasterTx               ((uint16_t)0x0200)
#define I2S_Mode_MasterRx               ((uint16_t)0x0300)

#define I2S_Standard_Phillips           ((uint16_t)0x0000)
#define I2S_Standard_MSB                ((uint16_t)0x0010)
#define I2S_Standard_LSB                ((uint16_t)0x0020)
#define I2S_Standard_PCMShort           ((uint16_t)0x0030)
#define I2S_Standard_PCMLong            ((uint16_t)0x00B0)

#define I2S_DataFormat_16b              ((uint16_t)0x0000)
#define I2S_DataFormat_16bextended      ((uint16_t)0x0001)
#define I2S_DataFormat_24b              ((uint16_t)0x0003)
#define I2S_DataFormat_32b              ((uint16_t)0x0005)

#define I2S_MCLKOutput_Enable           ((uint16_t)0x0200)
#define I2S_MCLKOutput_Disable          ((uint16_t)0x0000)

#define I2S_AudioFreq_192k               ((uint32_t)192000)
#define I2S_AudioFreq_96k                ((uint32_t)96000)
#define I2S_AudioFreq_48k                ((uint32_t)48000)
#define I2S_AudioFreq_44k                ((uint32_t)44100)
#define I2S_AudioFreq_32k                ((uint32_t)32000)
#define I2S_AudioFreq_22k                ((uint32_t)22050)
#define I2S_AudioFreq_16k                ((uint32_t)16000)
#define I2S_AudioFreq_11k                ((uint32_t)11025)
#define I2S_AudioFreq_8k                 ((uint32_t)8000)
#define I2S_AudioFreq_Default            ((uint32_t)2)

#define I2S_CPOL_Low                    ((uint16_t)0x0000)
#define I2S_CPOL_High                   ((uint16_t)0x0008)

#define SPI_I2S_DMAReq_Tx               ((uint16_t)0x0002)
#define SPI_I2S_DMAReq_Rx               ((uint16_t)0x0001)

#define SPI_NSSInternalSoft_Set         ((uint16_t)0x0100)
#define SPI_NSSInternalSoft_Reset       ((uint16_t)0xFEFF)

#define SPI_CRC_Tx                      ((uint8_t)0x00)
#define SPI_CRC_Rx                      ((uint8_t)0x01)

#define SPI_Direction_Rx                ((uint16_t)0xBFFF)
#define SPI_Direction_Tx                ((uint16_t)0x4000)

#define SPI_I2S_IT_TXE                  ((uint8_t)0x71)
#define SPI_I2S_IT_RXNE                 ((uint8_t)0x60)
#define SPI_I2S_IT_ERR                  ((uint8_t)0x50)

#define SPI_I2S_IT_OVR                  ((uint8_t)0x56)
#define SPI_IT_MODF                     ((uint8_t)0x55)
#define SPI_IT_CRCERR                   ((uint8_t)0x54)
#define I2S_IT_UDR                      ((uint8_t)0x53)


#define SPI_I2S_FLAG_RXNE               ((uint16_t)0x0001)
#define SPI_I2S_FLAG_TXE                ((uint16_t)0x0002)
#define I2S_FLAG_CHSIDE                 ((uint16_t)0x0004)
#define I2S_FLAG_UDR                    ((uint16_t)0x0008)
#define SPI_FLAG_CRCERR                 ((uint16_t)0x0010)
#define SPI_FLAG_MODF                   ((uint16_t)0x0020)
#define SPI_I2S_FLAG_OVR                ((uint16_t)0x0040)
#define SPI_I2S_FLAG_BSY                ((uint16_t)0x0080)


void SPI_I2S_DeInit(uint32_t SPIx);
void SPI_Init(uint32_t SPIx, SPI_InitTypeDef* SPI_InitStruct);
void I2S_Init(uint32_t SPIx, I2S_InitTypeDef* I2S_InitStruct);
void SPI_StructInit(SPI_InitTypeDef* SPI_InitStruct);
void I2S_StructInit(I2S_InitTypeDef* I2S_InitStruct);
void SPI_Cmd(uint32_t SPIx, FunctionalState NewState);
void I2S_Cmd(uint32_t SPIx, FunctionalState NewState);
void SPI_I2S_ITConfig(uint32_t SPIx, uint8_t SPI_I2S_IT, FunctionalState NewState);
void SPI_I2S_DMACmd(uint32_t SPIx, uint16_t SPI_I2S_DMAReq, FunctionalState NewState);
void SPI_I2S_SendData(uint32_t SPIx, uint16_t Data);
uint16_t SPI_I2S_ReceiveData(uint32_t SPIx);
void SPI_NSSInternalSoftwareConfig(uint32_t SPIx, uint16_t SPI_NSSInternalSoft);
void SPI_SSOutputCmd(uint32_t SPIx, FunctionalState NewState);
void SPI_DataSizeConfig(uint32_t SPIx, uint16_t SPI_DataSize);
void SPI_TransmitCRC(uint32_t SPIx);
void SPI_CalculateCRC(uint32_t SPIx, FunctionalState NewState);
uint16_t SPI_GetCRC(uint32_t SPIx, uint8_t SPI_CRC);
uint16_t SPI_GetCRCPolynomial(uint32_t SPIx);
void SPI_BiDirectionalLineConfig(uint32_t SPIx, uint16_t SPI_Direction);
FlagStatus SPI_I2S_GetFlagStatus(uint32_t SPIx, uint16_t SPI_I2S_FLAG);
void SPI_I2S_ClearFlag(uint32_t SPIx, uint16_t SPI_I2S_FLAG);
ITStatus SPI_I2S_GetITStatus(uint32_t SPIx, uint8_t SPI_I2S_IT);
void SPI_I2S_ClearITPendingBit(uint32_t SPIx, uint8_t SPI_I2S_IT);

#ifdef __cplusplus
}
#endif

#endif 

/********************END OF FILE****/
