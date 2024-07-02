#ifndef __SYSTEM_HC32F10X_H
#define __SYSTEM_HC32F10X_H

#ifdef __cplusplus
 extern "C" {
#endif 


extern unsigned SystemCoreClock;          /*!< System Clock Frequency (Core Clock) */


  
extern void SystemInit(void);
extern void SystemCoreClockUpdate(void);
/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif 


/*********************END OF FILE****/
