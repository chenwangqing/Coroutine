#ifndef __HC32F10x_WWDG_H
#define __HC32F10x_WWDG_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "hc32f10x.h"

/*************************************************************************
Register Address 
*************************************************************************/
#define WWDG									0x40002C00

#define WWDG_CR								(*(volatile uint32_t *)(uint32_t)(0x40002C00))
#define WWDG_CFR							(*(volatile uint32_t *)(uint32_t)(0x40002C04))
#define WWDG_SR								(*(volatile uint32_t *)(uint32_t)(0x40002C08))


#define WWDG_Prescaler_1    ((uint32_t)0x00000000)
#define WWDG_Prescaler_2    ((uint32_t)0x00000080)
#define WWDG_Prescaler_4    ((uint32_t)0x00000100)
#define WWDG_Prescaler_8    ((uint32_t)0x00000180)

  
void WWDG_DeInit(void);
void WWDG_SetPrescaler(uint32_t WWDG_Prescaler);
void WWDG_SetWindowValue(uint8_t WindowValue);
void WWDG_EnableIT(void);
void WWDG_SetCounter(uint8_t Counter);
void WWDG_Enable(uint8_t Counter);
FlagStatus WWDG_GetFlagStatus(void);
void WWDG_ClearFlag(void);

#ifdef __cplusplus
}
#endif

#endif



/***************END OF FILE****/
