#include "hc32f10x_dma.h"
#include "hc32f10x_rcc.h"



/* DMA1 Channelx interrupt pending bit masks */
#define DMA1_Channel1_IT_Mask    ((uint32_t)(DMA_ISR_GIF1 | DMA_ISR_TCIF1 | DMA_ISR_HTIF1 | DMA_ISR_TEIF1))
#define DMA1_Channel2_IT_Mask    ((uint32_t)(DMA_ISR_GIF2 | DMA_ISR_TCIF2 | DMA_ISR_HTIF2 | DMA_ISR_TEIF2))
#define DMA1_Channel3_IT_Mask    ((uint32_t)(DMA_ISR_GIF3 | DMA_ISR_TCIF3 | DMA_ISR_HTIF3 | DMA_ISR_TEIF3))
#define DMA1_Channel4_IT_Mask    ((uint32_t)(DMA_ISR_GIF4 | DMA_ISR_TCIF4 | DMA_ISR_HTIF4 | DMA_ISR_TEIF4))
#define DMA1_Channel5_IT_Mask    ((uint32_t)(DMA_ISR_GIF5 | DMA_ISR_TCIF5 | DMA_ISR_HTIF5 | DMA_ISR_TEIF5))
#define DMA1_Channel6_IT_Mask    ((uint32_t)(DMA_ISR_GIF6 | DMA_ISR_TCIF6 | DMA_ISR_HTIF6 | DMA_ISR_TEIF6))
#define DMA1_Channel7_IT_Mask    ((uint32_t)(DMA_ISR_GIF7 | DMA_ISR_TCIF7 | DMA_ISR_HTIF7 | DMA_ISR_TEIF7))

/* DMA2 Channelx interrupt pending bit masks */
#define DMA2_Channel1_IT_Mask    ((uint32_t)(DMA_ISR_GIF1 | DMA_ISR_TCIF1 | DMA_ISR_HTIF1 | DMA_ISR_TEIF1))
#define DMA2_Channel2_IT_Mask    ((uint32_t)(DMA_ISR_GIF2 | DMA_ISR_TCIF2 | DMA_ISR_HTIF2 | DMA_ISR_TEIF2))
#define DMA2_Channel3_IT_Mask    ((uint32_t)(DMA_ISR_GIF3 | DMA_ISR_TCIF3 | DMA_ISR_HTIF3 | DMA_ISR_TEIF3))
#define DMA2_Channel4_IT_Mask    ((uint32_t)(DMA_ISR_GIF4 | DMA_ISR_TCIF4 | DMA_ISR_HTIF4 | DMA_ISR_TEIF4))
#define DMA2_Channel5_IT_Mask    ((uint32_t)(DMA_ISR_GIF5 | DMA_ISR_TCIF5 | DMA_ISR_HTIF5 | DMA_ISR_TEIF5))

/* DMA2 FLAG mask */
#define FLAG_Mask                ((uint32_t)0x10000000)

/* DMA registers Masks */
#define CCR_CLEAR_Mask           ((uint32_t)0xFFFF800F)


/**
  * @brief  Deinitializes the DMAy Channelx registers to their default reset
  *         values.
  * @param  DMAx: where x can be 1 or 2 to select the DMA
  * @param  Channel: DMA_Channel1 | DMA_Channel2  | DMA_Channel3 | DMA_Channel4
									 | DMA_Channel5 | DMA_Channel6  | DMA_Channel7
  * @retval None
  */
void DMA_DeInit(uint32_t DMAx, uint32_t Channel)
{
  /* Disable the selected DMAy Channelx */
  DMA_CCR(DMAx, Channel) &= (uint16_t)(~DMA_CCR1_EN);
  
  /* Reset DMAy Channelx control register */
  DMA_CCR(DMAx, Channel)  = 0;
  
  /* Reset DMAy Channelx remaining bytes register */
  DMA_CNDTR(DMAx, Channel) = 0;
  
  /* Reset DMAy Channelx peripheral address register */
  DMA_CPAR(DMAx, Channel)  = 0;
  
  /* Reset DMAy Channelx memory address register */
  DMA_CMAR(DMAx, Channel) = 0;
	
  if (DMAx == DMA1)
  {
	  switch(Channel) 
	  {
			case DMA_Channel1:
				/* Reset interrupt pending bits for DMA1 Channel1 */
				DMA_IFCR(DMA1) |= DMA1_Channel1_IT_Mask;
				break;
			case DMA_Channel2:
				/* Reset interrupt pending bits for DMA1 Channel2 */
				DMA_IFCR(DMA1) |= DMA1_Channel2_IT_Mask;
				break;
			case DMA_Channel3:
				/* Reset interrupt pending bits for DMA1 Channel3 */
				DMA_IFCR(DMA1) |= DMA1_Channel3_IT_Mask;
				break;
			case DMA_Channel4:
				/* Reset interrupt pending bits for DMA1 Channel4 */
				DMA_IFCR(DMA1) |= DMA1_Channel4_IT_Mask;
				break;
			case DMA_Channel5:
				/* Reset interrupt pending bits for DMA1 Channel5 */
				DMA_IFCR(DMA1) |= DMA1_Channel5_IT_Mask;
				break;
			case DMA_Channel6:
				/* Reset interrupt pending bits for DMA1 Channel5 */
				DMA_IFCR(DMA1) |= DMA1_Channel6_IT_Mask;
				break;
			case DMA_Channel7:
				/* Reset interrupt pending bits for DMA1 Channel7 */
				DMA_IFCR(DMA1) |= DMA1_Channel7_IT_Mask;
				break;
			default:
				break;	  
	  }
	}
}

/**
  * @brief  Initializes the DMAy Channelx according to the specified
  *         parameters in the DMA_InitStruct.
  * @param  DMAx: where x can be 1 or 2 to select the DMA
  * @param  Channel: DMA_Channel1 | DMA_Channel2  | DMA_Channel3 | DMA_Channel4
									 | DMA_Channel5 | DMA_Channel6  | DMA_Channel7
  * @param  DMA_InitStruct: pointer to a DMA_InitTypeDef structure that
  *         contains the configuration information for the specified DMA Channel.
  * @retval None
  */
void DMA_Init(uint32_t DMAx, uint32_t Channel, DMA_InitTypeDef* DMA_InitStruct)
{
  uint32_t tmpreg = 0;

/*--------------------------- DMAy Channelx CCR Configuration -----------------*/
  /* Get the DMAy_Channelx CCR value */
  tmpreg = DMA_CCR(DMAx, Channel);
  /* Clear MEM2MEM, PL, MSIZE, PSIZE, MINC, PINC, CIRC and DIR bits */
  tmpreg &= CCR_CLEAR_Mask;
  /* Configure DMAy Channelx: data transfer, data size, priority level and mode */
  /* Set DIR bit according to DMA_DIR value */
  /* Set CIRC bit according to DMA_Mode value */
  /* Set PINC bit according to DMA_PeripheralInc value */
  /* Set MINC bit according to DMA_MemoryInc value */
  /* Set PSIZE bits according to DMA_PeripheralDataSize value */
  /* Set MSIZE bits according to DMA_MemoryDataSize value */
  /* Set PL bits according to DMA_Priority value */
  /* Set the MEM2MEM bit according to DMA_M2M value */
  tmpreg |= DMA_InitStruct->DMA_DIR | DMA_InitStruct->DMA_Mode |
            DMA_InitStruct->DMA_PeripheralInc | DMA_InitStruct->DMA_MemoryInc |
            DMA_InitStruct->DMA_PeripheralDataSize | DMA_InitStruct->DMA_MemoryDataSize |
            DMA_InitStruct->DMA_Priority | DMA_InitStruct->DMA_M2M;

  /* Write to DMAy Channelx CCR */
  DMA_CCR(DMAx, Channel) = tmpreg;

/*--------------------------- DMAy Channelx CNDTR Configuration ---------------*/
  /* Write to DMAy Channelx CNDTR */
  DMA_CNDTR(DMAx, Channel) = DMA_InitStruct->DMA_BufferSize;

/*--------------------------- DMAy Channelx CPAR Configuration ----------------*/
  /* Write to DMAy Channelx CPAR */
  DMA_CPAR(DMAx, Channel) = DMA_InitStruct->DMA_PeripheralBaseAddr;

/*--------------------------- DMAy Channelx CMAR Configuration ----------------*/
  /* Write to DMAy Channelx CMAR */
  DMA_CMAR(DMAx, Channel) = DMA_InitStruct->DMA_MemoryBaseAddr;
}

/**
  * @brief  Fills each DMA_InitStruct member with its default value.
  * @param  DMA_InitStruct : pointer to a DMA_InitTypeDef structure which will
  *         be initialized.
  * @retval None
  */
void DMA_StructInit(DMA_InitTypeDef* DMA_InitStruct)
{
/*-------------- Reset DMA init structure parameters values ------------------*/
  /* Initialize the DMA_PeripheralBaseAddr member */
  DMA_InitStruct->DMA_PeripheralBaseAddr = 0;
  /* Initialize the DMA_MemoryBaseAddr member */
  DMA_InitStruct->DMA_MemoryBaseAddr = 0;
  /* Initialize the DMA_DIR member */
  DMA_InitStruct->DMA_DIR = DMA_DIR_PeripheralSRC;
  /* Initialize the DMA_BufferSize member */
  DMA_InitStruct->DMA_BufferSize = 0;
  /* Initialize the DMA_PeripheralInc member */
  DMA_InitStruct->DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  /* Initialize the DMA_MemoryInc member */
  DMA_InitStruct->DMA_MemoryInc = DMA_MemoryInc_Disable;
  /* Initialize the DMA_PeripheralDataSize member */
  DMA_InitStruct->DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  /* Initialize the DMA_MemoryDataSize member */
  DMA_InitStruct->DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  /* Initialize the DMA_Mode member */
  DMA_InitStruct->DMA_Mode = DMA_Mode_Normal;
  /* Initialize the DMA_Priority member */
  DMA_InitStruct->DMA_Priority = DMA_Priority_Low;
  /* Initialize the DMA_M2M member */
  DMA_InitStruct->DMA_M2M = DMA_M2M_Disable;
}

/**
  * @brief  Enables or disables the specified DMAy Channelx.
  * @param  DMAx: where x can be 1 or 2 to select the DMA
  * @param  Channel: DMA_Channel1 | DMA_Channel2  | DMA_Channel3 | DMA_Channel4
									 | DMA_Channel5 | DMA_Channel6  | DMA_Channel7
  * @param  NewState: new state of the DMAy Channelx. 
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void DMA_Cmd(uint32_t DMAx, uint32_t Channel, FunctionalState NewState)
{
  if (NewState != DISABLE)
  {
    /* Enable the selected DMAy Channelx */
    DMA_CCR(DMAx, Channel) |= DMA_CCR1_EN;
  }
  else
  {
    /* Disable the selected DMAy Channelx */
    DMA_CCR(DMAx, Channel) &= (uint16_t)(~DMA_CCR1_EN);
  }
}

/**
  * @brief  Enables or disables the specified DMAy Channelx interrupts.
  * @param  DMAx: where x can be 1 or 2 to select the DMA
  * @param  Channel: DMA_Channel1 | DMA_Channel2  | DMA_Channel3 | DMA_Channel4
									 | DMA_Channel5 | DMA_Channel6  | DMA_Channel7
  * @param  DMA_IT: specifies the DMA interrupts sources to be enabled
  *   or disabled. 
  *   This parameter can be any combination of the following values:
  *     @arg DMA_IT_TC:  Transfer complete interrupt mask
  *     @arg DMA_IT_HT:  Half transfer interrupt mask
  *     @arg DMA_IT_TE:  Transfer error interrupt mask
  * @param  NewState: new state of the specified DMA interrupts.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void DMA_ITConfig(uint32_t DMAx, uint32_t Channel, uint32_t DMA_IT, FunctionalState NewState)
{
  if (NewState != DISABLE)
  {
    /* Enable the selected DMA interrupts */
    DMA_CCR(DMAx, Channel) |= DMA_IT;
  }
  else
  {
    /* Disable the selected DMA interrupts */
    DMA_CCR(DMAx, Channel) &= ~DMA_IT;
  }
}

/**
  * @brief  Sets the number of data units in the current DMAy Channelx transfer.
  * @param  DMAx: where x can be 1 or 2 to select the DMA
  * @param  Channel: DMA_Channel1 | DMA_Channel2  | DMA_Channel3 | DMA_Channel4
									 | DMA_Channel5 | DMA_Channel6  | DMA_Channel7
  * @param  DataNumber: The number of data units in the current DMAy Channelx
  *         transfer.   
  * @note   This function can only be used when the DMAy_Channelx is disabled.                 
  * @retval None.
  */
void DMA_SetCurrDataCounter(uint32_t DMAx, uint32_t Channel, uint16_t DataNumber)
{
  /* Write to DMAy Channelx CNDTR */
  DMA_CNDTR(DMAx, Channel) = DataNumber;  
}

/**
  * @brief  Returns the number of remaining data units in the current
  *         DMAy Channelx transfer.
  * @param  DMAx: where x can be 1 or 2 to select the DMA
  * @param  Channel: DMA_Channel1 | DMA_Channel2  | DMA_Channel3 | DMA_Channel4
									 | DMA_Channel5 | DMA_Channel6  | DMA_Channel7
  * @retval The number of remaining data units in the current DMAy Channelx
  *         transfer.
  */
uint16_t DMA_GetCurrDataCounter(uint32_t DMAx, uint32_t Channel)
{
  /* Return the number of remaining data units for DMAy Channelx */
  return ((uint16_t)(DMA_CNDTR(DMAx, Channel)));
}

/**
  * @brief  Checks whether the specified DMAy Channelx flag is set or not.
  * @param  DMAy_FLAG: specifies the flag to check.
  *   This parameter can be one of the following values:
  *     @arg DMA1_FLAG_GL1: DMA1 Channel1 global flag.
  *     @arg DMA1_FLAG_TC1: DMA1 Channel1 transfer complete flag.
  *     @arg DMA1_FLAG_HT1: DMA1 Channel1 half transfer flag.
  *     @arg DMA1_FLAG_TE1: DMA1 Channel1 transfer error flag.
  *     @arg DMA1_FLAG_GL2: DMA1 Channel2 global flag.
  *     @arg DMA1_FLAG_TC2: DMA1 Channel2 transfer complete flag.
  *     @arg DMA1_FLAG_HT2: DMA1 Channel2 half transfer flag.
  *     @arg DMA1_FLAG_TE2: DMA1 Channel2 transfer error flag.
  *     @arg DMA1_FLAG_GL3: DMA1 Channel3 global flag.
  *     @arg DMA1_FLAG_TC3: DMA1 Channel3 transfer complete flag.
  *     @arg DMA1_FLAG_HT3: DMA1 Channel3 half transfer flag.
  *     @arg DMA1_FLAG_TE3: DMA1 Channel3 transfer error flag.
  *     @arg DMA1_FLAG_GL4: DMA1 Channel4 global flag.
  *     @arg DMA1_FLAG_TC4: DMA1 Channel4 transfer complete flag.
  *     @arg DMA1_FLAG_HT4: DMA1 Channel4 half transfer flag.
  *     @arg DMA1_FLAG_TE4: DMA1 Channel4 transfer error flag.
  *     @arg DMA1_FLAG_GL5: DMA1 Channel5 global flag.
  *     @arg DMA1_FLAG_TC5: DMA1 Channel5 transfer complete flag.
  *     @arg DMA1_FLAG_HT5: DMA1 Channel5 half transfer flag.
  *     @arg DMA1_FLAG_TE5: DMA1 Channel5 transfer error flag.
  *     @arg DMA1_FLAG_GL6: DMA1 Channel6 global flag.
  *     @arg DMA1_FLAG_TC6: DMA1 Channel6 transfer complete flag.
  *     @arg DMA1_FLAG_HT6: DMA1 Channel6 half transfer flag.
  *     @arg DMA1_FLAG_TE6: DMA1 Channel6 transfer error flag.
  *     @arg DMA1_FLAG_GL7: DMA1 Channel7 global flag.
  *     @arg DMA1_FLAG_TC7: DMA1 Channel7 transfer complete flag.
  *     @arg DMA1_FLAG_HT7: DMA1 Channel7 half transfer flag.
  *     @arg DMA1_FLAG_TE7: DMA1 Channel7 transfer error flag.
  *     @arg DMA2_FLAG_GL1: DMA2 Channel1 global flag.
  *     @arg DMA2_FLAG_TC1: DMA2 Channel1 transfer complete flag.
  *     @arg DMA2_FLAG_HT1: DMA2 Channel1 half transfer flag.
  *     @arg DMA2_FLAG_TE1: DMA2 Channel1 transfer error flag.
  *     @arg DMA2_FLAG_GL2: DMA2 Channel2 global flag.
  *     @arg DMA2_FLAG_TC2: DMA2 Channel2 transfer complete flag.
  *     @arg DMA2_FLAG_HT2: DMA2 Channel2 half transfer flag.
  *     @arg DMA2_FLAG_TE2: DMA2 Channel2 transfer error flag.
  *     @arg DMA2_FLAG_GL3: DMA2 Channel3 global flag.
  *     @arg DMA2_FLAG_TC3: DMA2 Channel3 transfer complete flag.
  *     @arg DMA2_FLAG_HT3: DMA2 Channel3 half transfer flag.
  *     @arg DMA2_FLAG_TE3: DMA2 Channel3 transfer error flag.
  *     @arg DMA2_FLAG_GL4: DMA2 Channel4 global flag.
  *     @arg DMA2_FLAG_TC4: DMA2 Channel4 transfer complete flag.
  *     @arg DMA2_FLAG_HT4: DMA2 Channel4 half transfer flag.
  *     @arg DMA2_FLAG_TE4: DMA2 Channel4 transfer error flag.
  *     @arg DMA2_FLAG_GL5: DMA2 Channel5 global flag.
  *     @arg DMA2_FLAG_TC5: DMA2 Channel5 transfer complete flag.
  *     @arg DMA2_FLAG_HT5: DMA2 Channel5 half transfer flag.
  *     @arg DMA2_FLAG_TE5: DMA2 Channel5 transfer error flag.
  * @retval The new state of DMAy_FLAG (SET or RESET).
  */
FlagStatus DMA_GetFlagStatus(uint32_t DMAy_FLAG)
{
  FlagStatus bitstatus = RESET;
  uint32_t tmpreg = 0;
  
  /* Calculate the used DMAy */
  if ((DMAy_FLAG & FLAG_Mask) != (uint32_t)RESET)
  {
    /* Get DMA2 ISR register value */
    tmpreg = DMA_ISR(DMA2) ;
  }
  else
  {
    /* Get DMA1 ISR register value */
    tmpreg = DMA_ISR(DMA1) ;
  }

  /* Check the status of the specified DMAy flag */
  if ((tmpreg & DMAy_FLAG) != (uint32_t)RESET)
  {
    /* DMAy_FLAG is set */
    bitstatus = SET;
  }
  else
  {
    /* DMAy_FLAG is reset */
    bitstatus = RESET;
  }
  
  /* Return the DMAy_FLAG status */
  return  bitstatus;
}

/**
  * @brief  Clears the DMAy Channelx's pending flags.
  * @param  DMAy_FLAG: specifies the flag to clear.
  *   This parameter can be any combination (for the same DMA) of the following values:
  *     @arg DMA1_FLAG_GL1: DMA1 Channel1 global flag.
  *     @arg DMA1_FLAG_TC1: DMA1 Channel1 transfer complete flag.
  *     @arg DMA1_FLAG_HT1: DMA1 Channel1 half transfer flag.
  *     @arg DMA1_FLAG_TE1: DMA1 Channel1 transfer error flag.
  *     @arg DMA1_FLAG_GL2: DMA1 Channel2 global flag.
  *     @arg DMA1_FLAG_TC2: DMA1 Channel2 transfer complete flag.
  *     @arg DMA1_FLAG_HT2: DMA1 Channel2 half transfer flag.
  *     @arg DMA1_FLAG_TE2: DMA1 Channel2 transfer error flag.
  *     @arg DMA1_FLAG_GL3: DMA1 Channel3 global flag.
  *     @arg DMA1_FLAG_TC3: DMA1 Channel3 transfer complete flag.
  *     @arg DMA1_FLAG_HT3: DMA1 Channel3 half transfer flag.
  *     @arg DMA1_FLAG_TE3: DMA1 Channel3 transfer error flag.
  *     @arg DMA1_FLAG_GL4: DMA1 Channel4 global flag.
  *     @arg DMA1_FLAG_TC4: DMA1 Channel4 transfer complete flag.
  *     @arg DMA1_FLAG_HT4: DMA1 Channel4 half transfer flag.
  *     @arg DMA1_FLAG_TE4: DMA1 Channel4 transfer error flag.
  *     @arg DMA1_FLAG_GL5: DMA1 Channel5 global flag.
  *     @arg DMA1_FLAG_TC5: DMA1 Channel5 transfer complete flag.
  *     @arg DMA1_FLAG_HT5: DMA1 Channel5 half transfer flag.
  *     @arg DMA1_FLAG_TE5: DMA1 Channel5 transfer error flag.
  *     @arg DMA1_FLAG_GL6: DMA1 Channel6 global flag.
  *     @arg DMA1_FLAG_TC6: DMA1 Channel6 transfer complete flag.
  *     @arg DMA1_FLAG_HT6: DMA1 Channel6 half transfer flag.
  *     @arg DMA1_FLAG_TE6: DMA1 Channel6 transfer error flag.
  *     @arg DMA1_FLAG_GL7: DMA1 Channel7 global flag.
  *     @arg DMA1_FLAG_TC7: DMA1 Channel7 transfer complete flag.
  *     @arg DMA1_FLAG_HT7: DMA1 Channel7 half transfer flag.
  *     @arg DMA1_FLAG_TE7: DMA1 Channel7 transfer error flag.
  *     @arg DMA2_FLAG_GL1: DMA2 Channel1 global flag.
  *     @arg DMA2_FLAG_TC1: DMA2 Channel1 transfer complete flag.
  *     @arg DMA2_FLAG_HT1: DMA2 Channel1 half transfer flag.
  *     @arg DMA2_FLAG_TE1: DMA2 Channel1 transfer error flag.
  *     @arg DMA2_FLAG_GL2: DMA2 Channel2 global flag.
  *     @arg DMA2_FLAG_TC2: DMA2 Channel2 transfer complete flag.
  *     @arg DMA2_FLAG_HT2: DMA2 Channel2 half transfer flag.
  *     @arg DMA2_FLAG_TE2: DMA2 Channel2 transfer error flag.
  *     @arg DMA2_FLAG_GL3: DMA2 Channel3 global flag.
  *     @arg DMA2_FLAG_TC3: DMA2 Channel3 transfer complete flag.
  *     @arg DMA2_FLAG_HT3: DMA2 Channel3 half transfer flag.
  *     @arg DMA2_FLAG_TE3: DMA2 Channel3 transfer error flag.
  *     @arg DMA2_FLAG_GL4: DMA2 Channel4 global flag.
  *     @arg DMA2_FLAG_TC4: DMA2 Channel4 transfer complete flag.
  *     @arg DMA2_FLAG_HT4: DMA2 Channel4 half transfer flag.
  *     @arg DMA2_FLAG_TE4: DMA2 Channel4 transfer error flag.
  *     @arg DMA2_FLAG_GL5: DMA2 Channel5 global flag.
  *     @arg DMA2_FLAG_TC5: DMA2 Channel5 transfer complete flag.
  *     @arg DMA2_FLAG_HT5: DMA2 Channel5 half transfer flag.
  *     @arg DMA2_FLAG_TE5: DMA2 Channel5 transfer error flag.
  * @retval None
  */
void DMA_ClearFlag(uint32_t DMAy_FLAG)
{
  /* Calculate the used DMAy */
  if ((DMAy_FLAG & FLAG_Mask) != (uint32_t)RESET)
  {
    /* Clear the selected DMAy flags */
    DMA_IFCR(DMA2) = DMAy_FLAG;
  }
  else
  {
    /* Clear the selected DMAy flags */
    DMA_IFCR(DMA1) = DMAy_FLAG;
  }
}

/**
  * @brief  Checks whether the specified DMAy Channelx interrupt has occurred or not.
  * @param  DMAy_IT: specifies the DMAy interrupt source to check. 
  *   This parameter can be one of the following values:
  *     @arg DMA1_IT_GL1: DMA1 Channel1 global interrupt.
  *     @arg DMA1_IT_TC1: DMA1 Channel1 transfer complete interrupt.
  *     @arg DMA1_IT_HT1: DMA1 Channel1 half transfer interrupt.
  *     @arg DMA1_IT_TE1: DMA1 Channel1 transfer error interrupt.
  *     @arg DMA1_IT_GL2: DMA1 Channel2 global interrupt.
  *     @arg DMA1_IT_TC2: DMA1 Channel2 transfer complete interrupt.
  *     @arg DMA1_IT_HT2: DMA1 Channel2 half transfer interrupt.
  *     @arg DMA1_IT_TE2: DMA1 Channel2 transfer error interrupt.
  *     @arg DMA1_IT_GL3: DMA1 Channel3 global interrupt.
  *     @arg DMA1_IT_TC3: DMA1 Channel3 transfer complete interrupt.
  *     @arg DMA1_IT_HT3: DMA1 Channel3 half transfer interrupt.
  *     @arg DMA1_IT_TE3: DMA1 Channel3 transfer error interrupt.
  *     @arg DMA1_IT_GL4: DMA1 Channel4 global interrupt.
  *     @arg DMA1_IT_TC4: DMA1 Channel4 transfer complete interrupt.
  *     @arg DMA1_IT_HT4: DMA1 Channel4 half transfer interrupt.
  *     @arg DMA1_IT_TE4: DMA1 Channel4 transfer error interrupt.
  *     @arg DMA1_IT_GL5: DMA1 Channel5 global interrupt.
  *     @arg DMA1_IT_TC5: DMA1 Channel5 transfer complete interrupt.
  *     @arg DMA1_IT_HT5: DMA1 Channel5 half transfer interrupt.
  *     @arg DMA1_IT_TE5: DMA1 Channel5 transfer error interrupt.
  *     @arg DMA1_IT_GL6: DMA1 Channel6 global interrupt.
  *     @arg DMA1_IT_TC6: DMA1 Channel6 transfer complete interrupt.
  *     @arg DMA1_IT_HT6: DMA1 Channel6 half transfer interrupt.
  *     @arg DMA1_IT_TE6: DMA1 Channel6 transfer error interrupt.
  *     @arg DMA1_IT_GL7: DMA1 Channel7 global interrupt.
  *     @arg DMA1_IT_TC7: DMA1 Channel7 transfer complete interrupt.
  *     @arg DMA1_IT_HT7: DMA1 Channel7 half transfer interrupt.
  *     @arg DMA1_IT_TE7: DMA1 Channel7 transfer error interrupt.
  *     @arg DMA2_IT_GL1: DMA2 Channel1 global interrupt.
  *     @arg DMA2_IT_TC1: DMA2 Channel1 transfer complete interrupt.
  *     @arg DMA2_IT_HT1: DMA2 Channel1 half transfer interrupt.
  *     @arg DMA2_IT_TE1: DMA2 Channel1 transfer error interrupt.
  *     @arg DMA2_IT_GL2: DMA2 Channel2 global interrupt.
  *     @arg DMA2_IT_TC2: DMA2 Channel2 transfer complete interrupt.
  *     @arg DMA2_IT_HT2: DMA2 Channel2 half transfer interrupt.
  *     @arg DMA2_IT_TE2: DMA2 Channel2 transfer error interrupt.
  *     @arg DMA2_IT_GL3: DMA2 Channel3 global interrupt.
  *     @arg DMA2_IT_TC3: DMA2 Channel3 transfer complete interrupt.
  *     @arg DMA2_IT_HT3: DMA2 Channel3 half transfer interrupt.
  *     @arg DMA2_IT_TE3: DMA2 Channel3 transfer error interrupt.
  *     @arg DMA2_IT_GL4: DMA2 Channel4 global interrupt.
  *     @arg DMA2_IT_TC4: DMA2 Channel4 transfer complete interrupt.
  *     @arg DMA2_IT_HT4: DMA2 Channel4 half transfer interrupt.
  *     @arg DMA2_IT_TE4: DMA2 Channel4 transfer error interrupt.
  *     @arg DMA2_IT_GL5: DMA2 Channel5 global interrupt.
  *     @arg DMA2_IT_TC5: DMA2 Channel5 transfer complete interrupt.
  *     @arg DMA2_IT_HT5: DMA2 Channel5 half transfer interrupt.
  *     @arg DMA2_IT_TE5: DMA2 Channel5 transfer error interrupt.
  * @retval The new state of DMAy_IT (SET or RESET).
  */
ITStatus DMA_GetITStatus(uint32_t DMAy_IT)
{
  ITStatus bitstatus = RESET;
  uint32_t tmpreg = 0;

  /* Calculate the used DMA */
  if ((DMAy_IT & FLAG_Mask) != (uint32_t)RESET)
  {
    /* Get DMA2 ISR register value */
    tmpreg = DMA_ISR(DMA2);
  }
  else
  {
    /* Get DMA1 ISR register value */
    tmpreg = DMA_ISR(DMA1);
  }

  /* Check the status of the specified DMAy interrupt */
  if ((tmpreg & DMAy_IT) != (uint32_t)RESET)
  {
    /* DMAy_IT is set */
    bitstatus = SET;
  }
  else
  {
    /* DMAy_IT is reset */
    bitstatus = RESET;
  }
  /* Return the DMA_IT status */
  return  bitstatus;
}

/**
  * @brief  Clears the DMAy Channelx's interrupt pending bits.
  * @param  DMAy_IT: specifies the DMAy interrupt pending bit to clear.
  *   This parameter can be any combination (for the same DMA) of the following values:
  *     @arg DMA1_IT_GL1: DMA1 Channel1 global interrupt.
  *     @arg DMA1_IT_TC1: DMA1 Channel1 transfer complete interrupt.
  *     @arg DMA1_IT_HT1: DMA1 Channel1 half transfer interrupt.
  *     @arg DMA1_IT_TE1: DMA1 Channel1 transfer error interrupt.
  *     @arg DMA1_IT_GL2: DMA1 Channel2 global interrupt.
  *     @arg DMA1_IT_TC2: DMA1 Channel2 transfer complete interrupt.
  *     @arg DMA1_IT_HT2: DMA1 Channel2 half transfer interrupt.
  *     @arg DMA1_IT_TE2: DMA1 Channel2 transfer error interrupt.
  *     @arg DMA1_IT_GL3: DMA1 Channel3 global interrupt.
  *     @arg DMA1_IT_TC3: DMA1 Channel3 transfer complete interrupt.
  *     @arg DMA1_IT_HT3: DMA1 Channel3 half transfer interrupt.
  *     @arg DMA1_IT_TE3: DMA1 Channel3 transfer error interrupt.
  *     @arg DMA1_IT_GL4: DMA1 Channel4 global interrupt.
  *     @arg DMA1_IT_TC4: DMA1 Channel4 transfer complete interrupt.
  *     @arg DMA1_IT_HT4: DMA1 Channel4 half transfer interrupt.
  *     @arg DMA1_IT_TE4: DMA1 Channel4 transfer error interrupt.
  *     @arg DMA1_IT_GL5: DMA1 Channel5 global interrupt.
  *     @arg DMA1_IT_TC5: DMA1 Channel5 transfer complete interrupt.
  *     @arg DMA1_IT_HT5: DMA1 Channel5 half transfer interrupt.
  *     @arg DMA1_IT_TE5: DMA1 Channel5 transfer error interrupt.
  *     @arg DMA1_IT_GL6: DMA1 Channel6 global interrupt.
  *     @arg DMA1_IT_TC6: DMA1 Channel6 transfer complete interrupt.
  *     @arg DMA1_IT_HT6: DMA1 Channel6 half transfer interrupt.
  *     @arg DMA1_IT_TE6: DMA1 Channel6 transfer error interrupt.
  *     @arg DMA1_IT_GL7: DMA1 Channel7 global interrupt.
  *     @arg DMA1_IT_TC7: DMA1 Channel7 transfer complete interrupt.
  *     @arg DMA1_IT_HT7: DMA1 Channel7 half transfer interrupt.
  *     @arg DMA1_IT_TE7: DMA1 Channel7 transfer error interrupt.
  *     @arg DMA2_IT_GL1: DMA2 Channel1 global interrupt.
  *     @arg DMA2_IT_TC1: DMA2 Channel1 transfer complete interrupt.
  *     @arg DMA2_IT_HT1: DMA2 Channel1 half transfer interrupt.
  *     @arg DMA2_IT_TE1: DMA2 Channel1 transfer error interrupt.
  *     @arg DMA2_IT_GL2: DMA2 Channel2 global interrupt.
  *     @arg DMA2_IT_TC2: DMA2 Channel2 transfer complete interrupt.
  *     @arg DMA2_IT_HT2: DMA2 Channel2 half transfer interrupt.
  *     @arg DMA2_IT_TE2: DMA2 Channel2 transfer error interrupt.
  *     @arg DMA2_IT_GL3: DMA2 Channel3 global interrupt.
  *     @arg DMA2_IT_TC3: DMA2 Channel3 transfer complete interrupt.
  *     @arg DMA2_IT_HT3: DMA2 Channel3 half transfer interrupt.
  *     @arg DMA2_IT_TE3: DMA2 Channel3 transfer error interrupt.
  *     @arg DMA2_IT_GL4: DMA2 Channel4 global interrupt.
  *     @arg DMA2_IT_TC4: DMA2 Channel4 transfer complete interrupt.
  *     @arg DMA2_IT_HT4: DMA2 Channel4 half transfer interrupt.
  *     @arg DMA2_IT_TE4: DMA2 Channel4 transfer error interrupt.
  *     @arg DMA2_IT_GL5: DMA2 Channel5 global interrupt.
  *     @arg DMA2_IT_TC5: DMA2 Channel5 transfer complete interrupt.
  *     @arg DMA2_IT_HT5: DMA2 Channel5 half transfer interrupt.
  *     @arg DMA2_IT_TE5: DMA2 Channel5 transfer error interrupt.
  * @retval None
  */
void DMA_ClearITPendingBit(uint32_t DMAy_IT)
{
  /* Calculate the used DMAy */
  if ((DMAy_IT & FLAG_Mask) != (uint32_t)RESET)
  {
    /* Clear the selected DMAy interrupt pending bits */
    DMA_IFCR(DMA2) = DMAy_IT;
  }
  else
  {
    /* Clear the selected DMAy interrupt pending bits */
    DMA_IFCR(DMA1) = DMAy_IT;
  }
}


/*****END OF FILE****/
