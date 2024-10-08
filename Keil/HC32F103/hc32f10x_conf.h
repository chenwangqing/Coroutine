#ifndef __HC32F10x_CONF_H
#define __HC32F10x_CONF_H


#include "hc32f10x_adc.h"
#include "hc32f10x_bkp.h"
#include "hc32f10x_can.h"
#include "hc32f10x_crc.h"
#include "hc32f10x_dbgmcu.h"
#include "hc32f10x_dma.h"
#include "hc32f10x_exti.h"
#include "hc32f10x_flash.h"
#include "hc32f10x_gpio.h"
#include "hc32f10x_i2c.h"
#include "hc32f10x_iwdg.h"
#include "hc32f10x_pwr.h"
#include "hc32f10x_rcc.h"
#include "hc32f10x_rtc.h"
#include "hc32f10x_spi.h"
#include "hc32f10x_tim.h"
#include "hc32f10x_usart.h"
#include "hc32f10x_wwdg.h"
#include "misc.h" 

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Uncomment the line below to expanse the "assert_param" macro in the 
   Standard Peripheral Library drivers code */
/* #define USE_FULL_ASSERT    1 */

/* Exported macro ------------------------------------------------------------*/
#ifdef  USE_FULL_ASSERT

/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function which reports 
  *         the name of the source file and the source line number of the call 
  *         that failed. If expr is true, it returns no value.
  * @retval None
  */
  #define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0)
#endif /* USE_FULL_ASSERT */

#endif 

/*****END OF FILE****/
