
#ifndef __HC32F10x_ADC_H
#define __HC32F10x_ADC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "hc32f10x.h"

/*************************************************************************
Register Address 
*************************************************************************/
#define ADC1												0x40012400
#define ADC2												0x40012800

#define ADC_SR(ADCn)								(*(volatile uint32_t *)(uint32_t)(ADCn + 0x00))
#define ADC_CR1(ADCn)								(*(volatile uint32_t *)(uint32_t)(ADCn + 0x04))
#define ADC_CR2(ADCn)								(*(volatile uint32_t *)(uint32_t)(ADCn + 0x08))
#define ADC_SMPR1(ADCn)							(*(volatile uint32_t *)(uint32_t)(ADCn + 0x0C))
#define ADC_SMPR2(ADCn)							(*(volatile uint32_t *)(uint32_t)(ADCn + 0x10))
#define ADC_JOFR1(ADCn)							(*(volatile uint32_t *)(uint32_t)(ADCn + 0x14))
#define ADC_JOFR2(ADCn)							(*(volatile uint32_t *)(uint32_t)(ADCn + 0x18))
#define ADC_JOFR3(ADCn)							(*(volatile uint32_t *)(uint32_t)(ADCn + 0x1C))
#define ADC_JOFR4(ADCn)							(*(volatile uint32_t *)(uint32_t)(ADCn + 0x20))
#define ADC_HTR(ADCn)								(*(volatile uint32_t *)(uint32_t)(ADCn + 0x24))
#define ADC_LTR(ADCn)								(*(volatile uint32_t *)(uint32_t)(ADCn + 0x28))
#define ADC_SQR1(ADCn)							(*(volatile uint32_t *)(uint32_t)(ADCn + 0x2C))
#define ADC_SQR2(ADCn)							(*(volatile uint32_t *)(uint32_t)(ADCn + 0x30))
#define ADC_SQR3(ADCn)							(*(volatile uint32_t *)(uint32_t)(ADCn + 0x34))
#define ADC_JSQR(ADCn)							(*(volatile uint32_t *)(uint32_t)(ADCn + 0x38))
#define ADC_JDR1(ADCn)							(*(volatile uint32_t *)(uint32_t)(ADCn + 0x3C))
#define ADC_JDR2(ADCn)							(*(volatile uint32_t *)(uint32_t)(ADCn + 0x40))
#define ADC_JDR3(ADCn)							(*(volatile uint32_t *)(uint32_t)(ADCn + 0x44))
#define ADC_JDR4(ADCn)							(*(volatile uint32_t *)(uint32_t)(ADCn + 0x48))
#define ADC_DR(ADCn)								(*(volatile uint32_t *)(uint32_t)(ADCn + 0x4C))

typedef struct
{
  uint32_t ADC_Mode;                      /*!< Configures the ADC to operate in independent or
                                               dual mode. 
                                               This parameter can be a value of @ref ADC_mode */

  FunctionalState ADC_ScanConvMode;       /*!< Specifies whether the conversion is performed in
                                               Scan (multichannels) or Single (one channel) mode.
                                               This parameter can be set to ENABLE or DISABLE */

  FunctionalState ADC_ContinuousConvMode; /*!< Specifies whether the conversion is performed in
                                               Continuous or Single mode.
                                               This parameter can be set to ENABLE or DISABLE. */

  uint32_t ADC_ExternalTrigConv;          /*!< Defines the external trigger used to start the analog
                                               to digital conversion of regular channels. This parameter
                                               can be a value of @ref ADC_external_trigger_sources_for_regular_channels_conversion */

  uint32_t ADC_DataAlign;                 /*!< Specifies whether the ADC data alignment is left or right.
                                               This parameter can be a value of @ref ADC_data_align */

  uint8_t ADC_NbrOfChannel;               /*!< Specifies the number of ADC channels that will be converted
                                               using the sequencer for regular channel group.
                                               This parameter must range from 1 to 16. */
}ADC_InitTypeDef;


#define ADC_Mode_Independent                       ((uint32_t)0x00000000)
#define ADC_Mode_RegInjecSimult                    ((uint32_t)0x00010000)
#define ADC_Mode_RegSimult_AlterTrig               ((uint32_t)0x00020000)
#define ADC_Mode_InjecSimult_FastInterl            ((uint32_t)0x00030000)
#define ADC_Mode_InjecSimult_SlowInterl            ((uint32_t)0x00040000)
#define ADC_Mode_InjecSimult                       ((uint32_t)0x00050000)
#define ADC_Mode_RegSimult                         ((uint32_t)0x00060000)
#define ADC_Mode_FastInterl                        ((uint32_t)0x00070000)
#define ADC_Mode_SlowInterl                        ((uint32_t)0x00080000)
#define ADC_Mode_AlterTrig                         ((uint32_t)0x00090000)


#define ADC_ExternalTrigConv_T1_CC1                ((uint32_t)0x00000000) /*!< For ADC1 and ADC2 */
#define ADC_ExternalTrigConv_T1_CC2                ((uint32_t)0x00020000) /*!< For ADC1 and ADC2 */
#define ADC_ExternalTrigConv_T2_CC2                ((uint32_t)0x00060000) /*!< For ADC1 and ADC2 */
#define ADC_ExternalTrigConv_T3_TRGO               ((uint32_t)0x00080000) /*!< For ADC1 and ADC2 */
#define ADC_ExternalTrigConv_T4_CC4                ((uint32_t)0x000A0000) /*!< For ADC1 and ADC2 */
#define ADC_ExternalTrigConv_Ext_IT11_TIM8_TRGO    ((uint32_t)0x000C0000) /*!< For ADC1 and ADC2 */

#define ADC_ExternalTrigConv_T1_CC3                ((uint32_t)0x00040000) /*!< For ADC1, ADC2 and ADC3 */
#define ADC_ExternalTrigConv_None                  ((uint32_t)0x000E0000) /*!< For ADC1, ADC2 and ADC3 */

#define ADC_ExternalTrigConv_T3_CC1                ((uint32_t)0x00000000) /*!< For ADC3 only */
#define ADC_ExternalTrigConv_T2_CC3                ((uint32_t)0x00020000) /*!< For ADC3 only */
#define ADC_ExternalTrigConv_T8_CC1                ((uint32_t)0x00060000) /*!< For ADC3 only */
#define ADC_ExternalTrigConv_T8_TRGO               ((uint32_t)0x00080000) /*!< For ADC3 only */
#define ADC_ExternalTrigConv_T5_CC1                ((uint32_t)0x000A0000) /*!< For ADC3 only */
#define ADC_ExternalTrigConv_T5_CC3                ((uint32_t)0x000C0000) /*!< For ADC3 only */


#define ADC_DataAlign_Right                        ((uint32_t)0x00000000)
#define ADC_DataAlign_Left                         ((uint32_t)0x00000800)


#define ADC_Channel_0                               ((uint8_t)0x00)
#define ADC_Channel_1                               ((uint8_t)0x01)
#define ADC_Channel_2                               ((uint8_t)0x02)
#define ADC_Channel_3                               ((uint8_t)0x03)
#define ADC_Channel_4                               ((uint8_t)0x04)
#define ADC_Channel_5                               ((uint8_t)0x05)
#define ADC_Channel_6                               ((uint8_t)0x06)
#define ADC_Channel_7                               ((uint8_t)0x07)
#define ADC_Channel_8                               ((uint8_t)0x08)
#define ADC_Channel_9                               ((uint8_t)0x09)
#define ADC_Channel_10                              ((uint8_t)0x0A)
#define ADC_Channel_11                              ((uint8_t)0x0B)
#define ADC_Channel_12                              ((uint8_t)0x0C)
#define ADC_Channel_13                              ((uint8_t)0x0D)
#define ADC_Channel_14                              ((uint8_t)0x0E)
#define ADC_Channel_15                              ((uint8_t)0x0F)
#define ADC_Channel_16                              ((uint8_t)0x10)
#define ADC_Channel_17                              ((uint8_t)0x11)

#define ADC_Channel_TempSensor                      ((uint8_t)ADC_Channel_16)
#define ADC_Channel_Vrefint                         ((uint8_t)ADC_Channel_17)


#define ADC_SampleTime_1Cycles5                    ((uint8_t)0x00)
#define ADC_SampleTime_7Cycles5                    ((uint8_t)0x01)
#define ADC_SampleTime_13Cycles5                   ((uint8_t)0x02)
#define ADC_SampleTime_28Cycles5                   ((uint8_t)0x03)
#define ADC_SampleTime_41Cycles5                   ((uint8_t)0x04)
#define ADC_SampleTime_55Cycles5                   ((uint8_t)0x05)
#define ADC_SampleTime_71Cycles5                   ((uint8_t)0x06)
#define ADC_SampleTime_239Cycles5                  ((uint8_t)0x07)


#define ADC_ExternalTrigInjecConv_T2_TRGO           ((uint32_t)0x00002000) /*!< For ADC1 and ADC2 */
#define ADC_ExternalTrigInjecConv_T2_CC1            ((uint32_t)0x00003000) /*!< For ADC1 and ADC2 */
#define ADC_ExternalTrigInjecConv_T3_CC4            ((uint32_t)0x00004000) /*!< For ADC1 and ADC2 */
#define ADC_ExternalTrigInjecConv_T4_TRGO           ((uint32_t)0x00005000) /*!< For ADC1 and ADC2 */
#define ADC_ExternalTrigInjecConv_Ext_IT15_TIM8_CC4 ((uint32_t)0x00006000) /*!< For ADC1 and ADC2 */

#define ADC_ExternalTrigInjecConv_T1_TRGO           ((uint32_t)0x00000000) /*!< For ADC1, ADC2 and ADC3 */
#define ADC_ExternalTrigInjecConv_T1_CC4            ((uint32_t)0x00001000) /*!< For ADC1, ADC2 and ADC3 */
#define ADC_ExternalTrigInjecConv_None              ((uint32_t)0x00007000) /*!< For ADC1, ADC2 and ADC3 */

#define ADC_ExternalTrigInjecConv_T4_CC3            ((uint32_t)0x00002000) /*!< For ADC3 only */
#define ADC_ExternalTrigInjecConv_T8_CC2            ((uint32_t)0x00003000) /*!< For ADC3 only */
#define ADC_ExternalTrigInjecConv_T8_CC4            ((uint32_t)0x00004000) /*!< For ADC3 only */
#define ADC_ExternalTrigInjecConv_T5_TRGO           ((uint32_t)0x00005000) /*!< For ADC3 only */
#define ADC_ExternalTrigInjecConv_T5_CC4            ((uint32_t)0x00006000) /*!< For ADC3 only */


#define ADC_InjectedChannel_1                       ((uint8_t)0x14)
#define ADC_InjectedChannel_2                       ((uint8_t)0x18)
#define ADC_InjectedChannel_3                       ((uint8_t)0x1C)
#define ADC_InjectedChannel_4                       ((uint8_t)0x20)


#define ADC_AnalogWatchdog_SingleRegEnable         ((uint32_t)0x00800200)
#define ADC_AnalogWatchdog_SingleInjecEnable       ((uint32_t)0x00400200)
#define ADC_AnalogWatchdog_SingleRegOrInjecEnable  ((uint32_t)0x00C00200)
#define ADC_AnalogWatchdog_AllRegEnable            ((uint32_t)0x00800000)
#define ADC_AnalogWatchdog_AllInjecEnable          ((uint32_t)0x00400000)
#define ADC_AnalogWatchdog_AllRegAllInjecEnable    ((uint32_t)0x00C00000)
#define ADC_AnalogWatchdog_None                    ((uint32_t)0x00000000)


#define ADC_IT_EOC                                 ((uint16_t)0x0220)
#define ADC_IT_AWD                                 ((uint16_t)0x0140)
#define ADC_IT_JEOC                                ((uint16_t)0x0480)


#define ADC_FLAG_AWD                               ((uint8_t)0x01)
#define ADC_FLAG_EOC                               ((uint8_t)0x02)
#define ADC_FLAG_JEOC                              ((uint8_t)0x04)
#define ADC_FLAG_JSTRT                             ((uint8_t)0x08)
#define ADC_FLAG_STRT                              ((uint8_t)0x10)


void ADC_DeInit(uint32_t ADCx);
void ADC_Init(uint32_t ADCx, ADC_InitTypeDef* ADC_InitStruct);
void ADC_StructInit(ADC_InitTypeDef* ADC_InitStruct);
void ADC_Cmd(uint32_t ADCx, FunctionalState NewState);
void ADC_DMACmd(uint32_t ADCx, FunctionalState NewState);
void ADC_ITConfig(uint32_t ADCx, uint16_t ADC_IT, FunctionalState NewState);
void ADC_SoftwareStartConvCmd(uint32_t ADCx, FunctionalState NewState);
FlagStatus ADC_GetSoftwareStartConvStatus(uint32_t ADCx);
void ADC_DiscModeChannelCountConfig(uint32_t ADCx, uint8_t Number);
void ADC_DiscModeCmd(uint32_t ADCx, FunctionalState NewState);
void ADC_RegularChannelConfig(uint32_t ADCx, uint8_t ADC_Channel, uint8_t Rank, uint8_t ADC_SampleTime);
void ADC_ExternalTrigConvCmd(uint32_t ADCx, FunctionalState NewState);
uint16_t ADC_GetConversionValue(uint32_t ADCx);
uint32_t ADC_GetDualModeConversionValue(void);
void ADC_AutoInjectedConvCmd(uint32_t ADCx, FunctionalState NewState);
void ADC_InjectedDiscModeCmd(uint32_t ADCx, FunctionalState NewState);
void ADC_ExternalTrigInjectedConvConfig(uint32_t ADCx, uint32_t ADC_ExternalTrigInjecConv);
void ADC_ExternalTrigInjectedConvCmd(uint32_t ADCx, FunctionalState NewState);
void ADC_SoftwareStartInjectedConvCmd(uint32_t ADCx, FunctionalState NewState);
FlagStatus ADC_GetSoftwareStartInjectedConvCmdStatus(uint32_t ADCx);
void ADC_InjectedChannelConfig(uint32_t ADCx, uint8_t ADC_Channel, uint8_t Rank, uint8_t ADC_SampleTime);
void ADC_InjectedSequencerLengthConfig(uint32_t ADCx, uint8_t Length);
void ADC_SetInjectedOffset(uint32_t ADCx, uint8_t ADC_InjectedChannel, uint16_t Offset);
uint16_t ADC_GetInjectedConversionValue(uint32_t ADCx, uint8_t ADC_InjectedChannel);
void ADC_AnalogWatchdogCmd(uint32_t ADCx, uint32_t ADC_AnalogWatchdog);
void ADC_AnalogWatchdogThresholdsConfig(uint32_t ADCx, uint16_t HighThreshold, uint16_t LowThreshold);
void ADC_AnalogWatchdogSingleChannelConfig(uint32_t ADCx, uint8_t ADC_Channel);
void ADC_TempSensorVrefintCmd(FunctionalState NewState);
FlagStatus ADC_GetFlagStatus(uint32_t ADCx, uint8_t ADC_FLAG);
void ADC_ClearFlag(uint32_t ADCx, uint8_t ADC_FLAG);
ITStatus ADC_GetITStatus(uint32_t ADCx, uint16_t ADC_IT);
void ADC_ClearITPendingBit(uint32_t ADCx, uint16_t ADC_IT);

#ifdef __cplusplus
}
#endif

#endif


/*****END OF FILE****/
