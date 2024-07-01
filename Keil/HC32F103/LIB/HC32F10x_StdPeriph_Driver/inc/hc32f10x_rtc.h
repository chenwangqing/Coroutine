#ifndef __HC32F10x_RTC_H
#define __HC32F10x_RTC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "hc32f10x.h"

/*************************************************************************
Register Address 
*************************************************************************/
#define RTC_CRH								(*(volatile uint32_t *)(0x40002800))
#define RTC_CRL								(*(volatile uint32_t *)(0x40002804))
#define RTC_PRLH							(*(volatile uint32_t *)(0x40002808))
#define RTC_PRLL							(*(volatile uint32_t *)(0x4000280C))
#define RTC_DIVH							(*(volatile uint32_t *)(0x40002810))
#define RTC_DIVL							(*(volatile uint32_t *)(0x40002814))
#define RTC_CNTH							(*(volatile uint32_t *)(0x40002818))
#define RTC_CNTL							(*(volatile uint32_t *)(0x4000281C))
#define RTC_ALRH							(*(volatile uint32_t *)(0x40002820))
#define RTC_ALRL							(*(volatile uint32_t *)(0x40002824))
	

#define RTC_IT_OW            ((uint16_t)0x0004)  /*!< Overflow interrupt */
#define RTC_IT_ALR           ((uint16_t)0x0002)  /*!< Alarm interrupt */
#define RTC_IT_SEC           ((uint16_t)0x0001)  /*!< Second interrupt */


#define RTC_FLAG_RTOFF       ((uint16_t)0x0020)  /*!< RTC Operation OFF flag */
#define RTC_FLAG_RSF         ((uint16_t)0x0008)  /*!< Registers Synchronized flag */
#define RTC_FLAG_OW          ((uint16_t)0x0004)  /*!< Overflow flag */
#define RTC_FLAG_ALR         ((uint16_t)0x0002)  /*!< Alarm flag */
#define RTC_FLAG_SEC         ((uint16_t)0x0001)  /*!< Second flag */

void RTC_ITConfig(uint16_t RTC_IT, FunctionalState NewState);
void RTC_EnterConfigMode(void);
void RTC_ExitConfigMode(void);
uint32_t  RTC_GetCounter(void);
void RTC_SetCounter(uint32_t CounterValue);
void RTC_SetPrescaler(uint32_t PrescalerValue);
void RTC_SetAlarm(uint32_t AlarmValue);
uint32_t  RTC_GetDivider(void);
void RTC_WaitForLastTask(void);
void RTC_WaitForSynchro(void);
FlagStatus RTC_GetFlagStatus(uint16_t RTC_FLAG);
void RTC_ClearFlag(uint16_t RTC_FLAG);
ITStatus RTC_GetITStatus(uint16_t RTC_IT);
void RTC_ClearITPendingBit(uint16_t RTC_IT);

#ifdef __cplusplus
}
#endif

#endif 

/*****************END OF FILE****/
