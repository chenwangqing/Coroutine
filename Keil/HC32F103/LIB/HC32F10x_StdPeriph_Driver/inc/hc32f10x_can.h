#ifndef __HC32F10x_CAN_H
#define __HC32F10x_CAN_H

#ifdef __cplusplus
 extern "C" {
#endif


#include "hc32f10x.h"



typedef struct
{
  uint16_t CAN_Prescaler;   /*!< Specifies the length of a time quantum. 
                                 It ranges from 1 to 1024. */
  
  uint8_t CAN_Mode;         /*!< Specifies the CAN operating mode.
                                 This parameter can be a value of 
                                @ref CAN_operating_mode */

  uint8_t CAN_SJW;          /*!< Specifies the maximum number of time quanta 
                                 the CAN hardware is allowed to lengthen or 
                                 shorten a bit to perform resynchronization.
                                 This parameter can be a value of 
                                 @ref CAN_synchronisation_jump_width */

  uint8_t CAN_BS1;          /*!< Specifies the number of time quanta in Bit 
                                 Segment 1. This parameter can be a value of 
                                 @ref CAN_time_quantum_in_bit_segment_1 */

  uint8_t CAN_BS2;          /*!< Specifies the number of time quanta in Bit 
                                 Segment 2.
                                 This parameter can be a value of 
                                 @ref CAN_time_quantum_in_bit_segment_2 */
  
  FunctionalState CAN_TTCM; /*!< Enable or disable the time triggered 
                                 communication mode. This parameter can be set 
                                 either to ENABLE or DISABLE. */
  
  FunctionalState CAN_ABOM;  /*!< Enable or disable the automatic bus-off 
                                  management. This parameter can be set either 
                                  to ENABLE or DISABLE. */

  FunctionalState CAN_AWUM;  /*!< Enable or disable the automatic wake-up mode. 
                                  This parameter can be set either to ENABLE or 
                                  DISABLE. */

  FunctionalState CAN_NART;  /*!< Enable or disable the no-automatic 
                                  retransmission mode. This parameter can be 
                                  set either to ENABLE or DISABLE. */

  FunctionalState CAN_RFLM;  /*!< Enable or disable the Receive FIFO Locked mode.
                                  This parameter can be set either to ENABLE 
                                  or DISABLE. */

  FunctionalState CAN_TXFP;  /*!< Enable or disable the transmit FIFO priority.
                                  This parameter can be set either to ENABLE 
                                  or DISABLE. */
} CAN_InitTypeDef;

/** 
  * @brief  CAN filter init structure definition
  */

typedef struct
{
  uint16_t CAN_FilterIdHigh;         /*!< Specifies the filter identification number (MSBs for a 32-bit
                                              configuration, first one for a 16-bit configuration).
                                              This parameter can be a value between 0x0000 and 0xFFFF */

  uint16_t CAN_FilterIdLow;          /*!< Specifies the filter identification number (LSBs for a 32-bit
                                              configuration, second one for a 16-bit configuration).
                                              This parameter can be a value between 0x0000 and 0xFFFF */

  uint16_t CAN_FilterMaskIdHigh;     /*!< Specifies the filter mask number or identification number,
                                              according to the mode (MSBs for a 32-bit configuration,
                                              first one for a 16-bit configuration).
                                              This parameter can be a value between 0x0000 and 0xFFFF */

  uint16_t CAN_FilterMaskIdLow;      /*!< Specifies the filter mask number or identification number,
                                              according to the mode (LSBs for a 32-bit configuration,
                                              second one for a 16-bit configuration).
                                              This parameter can be a value between 0x0000 and 0xFFFF */

  uint16_t CAN_FilterFIFOAssignment; /*!< Specifies the FIFO (0 or 1) which will be assigned to the filter.
                                              This parameter can be a value of @ref CAN_filter_FIFO */
  
  uint8_t CAN_FilterNumber;          /*!< Specifies the filter which will be initialized. It ranges from 0 to 13. */

  uint8_t CAN_FilterMode;            /*!< Specifies the filter mode to be initialized.
                                              This parameter can be a value of @ref CAN_filter_mode */

  uint8_t CAN_FilterScale;           /*!< Specifies the filter scale.
                                              This parameter can be a value of @ref CAN_filter_scale */

  FunctionalState CAN_FilterActivation; /*!< Enable or disable the filter.
                                              This parameter can be set either to ENABLE or DISABLE. */
} CAN_FilterInitTypeDef;

/** 
  * @brief  CAN Tx message structure definition  
  */

typedef struct
{
  uint32_t StdId;  /*!< Specifies the standard identifier.
                        This parameter can be a value between 0 to 0x7FF. */

  uint32_t ExtId;  /*!< Specifies the extended identifier.
                        This parameter can be a value between 0 to 0x1FFFFFFF. */

  uint8_t IDE;     /*!< Specifies the type of identifier for the message that 
                        will be transmitted. This parameter can be a value 
                        of @ref CAN_identifier_type */

  uint8_t RTR;     /*!< Specifies the type of frame for the message that will 
                        be transmitted. This parameter can be a value of 
                        @ref CAN_remote_transmission_request */

  uint8_t DLC;     /*!< Specifies the length of the frame that will be 
                        transmitted. This parameter can be a value between 
                        0 to 8 */

  uint8_t Data[8]; /*!< Contains the data to be transmitted. It ranges from 0 
                        to 0xFF. */
} CanTxMsg;

/** 
  * @brief  CAN Rx message structure definition  
  */

typedef struct
{
  uint32_t StdId;  /*!< Specifies the standard identifier.
                        This parameter can be a value between 0 to 0x7FF. */

  uint32_t ExtId;  /*!< Specifies the extended identifier.
                        This parameter can be a value between 0 to 0x1FFFFFFF. */

  uint8_t IDE;     /*!< Specifies the type of identifier for the message that 
                        will be received. This parameter can be a value of 
                        @ref CAN_identifier_type */

  uint8_t RTR;     /*!< Specifies the type of frame for the received message.
                        This parameter can be a value of 
                        @ref CAN_remote_transmission_request */

  uint8_t DLC;     /*!< Specifies the length of the frame that will be received.
                        This parameter can be a value between 0 to 8 */

  uint8_t Data[8]; /*!< Contains the data to be received. It ranges from 0 to 
                        0xFF. */

  uint8_t FMI;     /*!< Specifies the index of the filter the message stored in 
                        the mailbox passes through. This parameter can be a 
                        value between 0 to 0xFF */
} CanRxMsg;


/*************************************************************************
Register Address 
*************************************************************************/
#define CAN1													0x40006400
#define CAN2													0x40006800

#define CAN1_FiR1_BASE								0x40006640
#define CAN1_FiR2_BASE								0x40006644

#define CAN_TIR_OFFSET_BASE 					0x180
#define CAN_TDTR_OFFSET_BASE 					0x184
#define CAN_TDLR_OFFSET_BASE 					0x188
#define CAN_TDHR_OFFSET_BASE 					0x18C

#define CAN_RIR_OFFSET_BASE 					0x1B0
#define CAN_RDTR_OFFSET_BASE 					0x1B4
#define CAN_RDLR_OFFSET_BASE 					0x1B8
#define CAN_RDHR_OFFSET_BASE 					0x1BC

#define CAN_MCR(CANn)									(*(volatile uint32_t *)(uint32_t)(CANn + 0x00))
#define CAN_MSR(CANn)									(*(volatile uint32_t *)(uint32_t)(CANn + 0x04))
#define CAN_TSR(CANn)									(*(volatile uint32_t *)(uint32_t)(CANn + 0x08))
#define CAN_RF0R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x0C))
#define CAN_RF1R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x10))
#define CAN_IER(CANn)									(*(volatile uint32_t *)(uint32_t)(CANn + 0x14))
#define CAN_ESR(CANn)									(*(volatile uint32_t *)(uint32_t)(CANn + 0x18))
#define CAN_BTR(CANn)									(*(volatile uint32_t *)(uint32_t)(CANn + 0x1C))

#define CAN_TI0R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x180))
#define CAN_TI1R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x190))
#define CAN_TI2R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x1A0))
#define CAN_TDT0R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x184))
#define CAN_TDT1R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x194))
#define CAN_TDT2R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x1A4))
#define CAN_TDL0R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x188))
#define CAN_TDL1R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x198))
#define CAN_TDL2R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x1A8))
#define CAN_TDH0R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x18C))
#define CAN_TDH1R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x19C))
#define CAN_TDH2R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x1AC))

#define CAN_RI0R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x1B0))
#define CAN_RI1R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x1C0))
#define CAN_RDT0R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x1B4))
#define CAN_RDT1R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x1C4))
#define CAN_RDL0R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x1B8))
#define CAN_RDL1R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x1C8))
#define CAN_RDH0R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x1BC))
#define CAN_RDH1R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x1CC))

#define CAN_FMR(CANn)									(*(volatile uint32_t *)(uint32_t)(CANn + 0x200))
#define CAN_FM1R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x204))
#define CAN_FS1R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x20C))
#define CAN_FFA1R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x214))
#define CAN_FA1R(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x21C))
#define CAN_F0R1(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x240))
#define CAN_F0R2(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x244))
#define CAN_F1R1(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x248))
#define CAN_F1R2(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x24C))
#define CAN_F2R1(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x250))
#define CAN_F2R2(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x254))
#define CAN_F3R1(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x258))
#define CAN_F3R2(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x25C))
#define CAN_F4R1(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x260))
#define CAN_F4R2(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x264))
#define CAN_F5R1(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x268))
#define CAN_F5R2(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x26C))
#define CAN_F6R1(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x270))
#define CAN_F6R2(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x274))
#define CAN_F7R1(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x278))
#define CAN_F7R2(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x27C))
#define CAN_F8R1(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x280))
#define CAN_F8R2(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x284))
#define CAN_F9R1(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x288))
#define CAN_F9R2(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x28C))
#define CAN_F10R1(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x290))
#define CAN_F10R2(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x294))
#define CAN_F11R1(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x298))
#define CAN_F11R2(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x29C))
#define CAN_F12R1(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x2A0))
#define CAN_F12R2(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x2A4))
#define CAN_F13R1(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x2A8))
#define CAN_F13R2(CANn)								(*(volatile uint32_t *)(uint32_t)(CANn + 0x2AC))

#define CAN_InitStatus_Failed              ((uint8_t)0x00) /*!< CAN initialization failed */
#define CAN_InitStatus_Success             ((uint8_t)0x01) /*!< CAN initialization OK */


#define CAN_Mode_Normal             ((uint8_t)0x00)  /*!< normal mode */
#define CAN_Mode_LoopBack           ((uint8_t)0x01)  /*!< loopback mode */
#define CAN_Mode_Silent             ((uint8_t)0x02)  /*!< silent mode */
#define CAN_Mode_Silent_LoopBack    ((uint8_t)0x03)  /*!< loopback combined with silent mode */


#define CAN_OperatingMode_Initialization  ((uint8_t)0x00) /*!< Initialization mode */
#define CAN_OperatingMode_Normal          ((uint8_t)0x01) /*!< Normal mode */
#define CAN_OperatingMode_Sleep           ((uint8_t)0x02) /*!< sleep mode */


#define CAN_ModeStatus_Failed    ((uint8_t)0x00)                /*!< CAN entering the specific mode failed */
#define CAN_ModeStatus_Success   ((uint8_t)!CAN_ModeStatus_Failed)   /*!< CAN entering the specific mode Succeed */


#define CAN_SJW_1tq                 ((uint8_t)0x00)  /*!< 1 time quantum */
#define CAN_SJW_2tq                 ((uint8_t)0x01)  /*!< 2 time quantum */
#define CAN_SJW_3tq                 ((uint8_t)0x02)  /*!< 3 time quantum */
#define CAN_SJW_4tq                 ((uint8_t)0x03)  /*!< 4 time quantum */


#define CAN_BS1_1tq                 ((uint8_t)0x00)  /*!< 1 time quantum */
#define CAN_BS1_2tq                 ((uint8_t)0x01)  /*!< 2 time quantum */
#define CAN_BS1_3tq                 ((uint8_t)0x02)  /*!< 3 time quantum */
#define CAN_BS1_4tq                 ((uint8_t)0x03)  /*!< 4 time quantum */
#define CAN_BS1_5tq                 ((uint8_t)0x04)  /*!< 5 time quantum */
#define CAN_BS1_6tq                 ((uint8_t)0x05)  /*!< 6 time quantum */
#define CAN_BS1_7tq                 ((uint8_t)0x06)  /*!< 7 time quantum */
#define CAN_BS1_8tq                 ((uint8_t)0x07)  /*!< 8 time quantum */
#define CAN_BS1_9tq                 ((uint8_t)0x08)  /*!< 9 time quantum */
#define CAN_BS1_10tq                ((uint8_t)0x09)  /*!< 10 time quantum */
#define CAN_BS1_11tq                ((uint8_t)0x0A)  /*!< 11 time quantum */
#define CAN_BS1_12tq                ((uint8_t)0x0B)  /*!< 12 time quantum */
#define CAN_BS1_13tq                ((uint8_t)0x0C)  /*!< 13 time quantum */
#define CAN_BS1_14tq                ((uint8_t)0x0D)  /*!< 14 time quantum */
#define CAN_BS1_15tq                ((uint8_t)0x0E)  /*!< 15 time quantum */
#define CAN_BS1_16tq                ((uint8_t)0x0F)  /*!< 16 time quantum */


#define CAN_BS2_1tq                 ((uint8_t)0x00)  /*!< 1 time quantum */
#define CAN_BS2_2tq                 ((uint8_t)0x01)  /*!< 2 time quantum */
#define CAN_BS2_3tq                 ((uint8_t)0x02)  /*!< 3 time quantum */
#define CAN_BS2_4tq                 ((uint8_t)0x03)  /*!< 4 time quantum */
#define CAN_BS2_5tq                 ((uint8_t)0x04)  /*!< 5 time quantum */
#define CAN_BS2_6tq                 ((uint8_t)0x05)  /*!< 6 time quantum */
#define CAN_BS2_7tq                 ((uint8_t)0x06)  /*!< 7 time quantum */
#define CAN_BS2_8tq                 ((uint8_t)0x07)  /*!< 8 time quantum */


#define CAN_FilterMode_IdMask       ((uint8_t)0x00)  /*!< identifier/mask mode */
#define CAN_FilterMode_IdList       ((uint8_t)0x01)  /*!< identifier list mode */


#define CAN_FilterScale_16bit       ((uint8_t)0x00) /*!< Two 16-bit filters */
#define CAN_FilterScale_32bit       ((uint8_t)0x01) /*!< One 32-bit filter */


#define CAN_Filter_FIFO0             ((uint8_t)0x00)  /*!< Filter FIFO 0 assignment for filter x */
#define CAN_Filter_FIFO1             ((uint8_t)0x01)  /*!< Filter FIFO 1 assignment for filter x */


#define CAN_Id_Standard             ((uint32_t)0x00000000)  /*!< Standard Id */
#define CAN_Id_Extended             ((uint32_t)0x00000004)  /*!< Extended Id */


#define CAN_RTR_Data                ((uint32_t)0x00000000)  /*!< Data frame */
#define CAN_RTR_Remote              ((uint32_t)0x00000002)  /*!< Remote frame */


#define CAN_TxStatus_Failed         ((uint8_t)0x00)/*!< CAN transmission failed */
#define CAN_TxStatus_Ok             ((uint8_t)0x01) /*!< CAN transmission succeeded */
#define CAN_TxStatus_Pending        ((uint8_t)0x02) /*!< CAN transmission pending */
#define CAN_TxStatus_NoMailBox      ((uint8_t)0x04) /*!< CAN cell did not provide an empty mailbox */


#define CAN_FIFO0                 ((uint8_t)0x00) /*!< CAN FIFO 0 used to receive */
#define CAN_FIFO1                 ((uint8_t)0x01) /*!< CAN FIFO 1 used to receive */


#define CAN_Sleep_Failed     ((uint8_t)0x00) /*!< CAN did not enter the sleep mode */
#define CAN_Sleep_Ok         ((uint8_t)0x01) /*!< CAN entered the sleep mode */


#define CAN_WakeUp_Failed        ((uint8_t)0x00) /*!< CAN did not leave the sleep mode */
#define CAN_WakeUp_Ok            ((uint8_t)0x01) /*!< CAN leaved the sleep mode */

                                                              
#define CAN_ErrorCode_NoErr           ((uint8_t)0x00) /*!< No Error */ 
#define	CAN_ErrorCode_StuffErr        ((uint8_t)0x10) /*!< Stuff Error */ 
#define	CAN_ErrorCode_FormErr         ((uint8_t)0x20) /*!< Form Error */ 
#define	CAN_ErrorCode_ACKErr          ((uint8_t)0x30) /*!< Acknowledgment Error */ 
#define	CAN_ErrorCode_BitRecessiveErr ((uint8_t)0x40) /*!< Bit Recessive Error */ 
#define	CAN_ErrorCode_BitDominantErr  ((uint8_t)0x50) /*!< Bit Dominant Error */ 
#define	CAN_ErrorCode_CRCErr          ((uint8_t)0x60) /*!< CRC Error  */ 
#define	CAN_ErrorCode_SoftwareSetErr  ((uint8_t)0x70) /*!< Software Set Error */ 


/* Transmit Flags */
#define CAN_FLAG_RQCP0             ((uint32_t)0x38000001) /*!< Request MailBox0 Flag */
#define CAN_FLAG_RQCP1             ((uint32_t)0x38000100) /*!< Request MailBox1 Flag */
#define CAN_FLAG_RQCP2             ((uint32_t)0x38010000) /*!< Request MailBox2 Flag */

/* Receive Flags */
#define CAN_FLAG_FMP0              ((uint32_t)0x12000003) /*!< FIFO 0 Message Pending Flag */
#define CAN_FLAG_FF0               ((uint32_t)0x32000008) /*!< FIFO 0 Full Flag            */
#define CAN_FLAG_FOV0              ((uint32_t)0x32000010) /*!< FIFO 0 Overrun Flag         */
#define CAN_FLAG_FMP1              ((uint32_t)0x14000003) /*!< FIFO 1 Message Pending Flag */
#define CAN_FLAG_FF1               ((uint32_t)0x34000008) /*!< FIFO 1 Full Flag            */
#define CAN_FLAG_FOV1              ((uint32_t)0x34000010) /*!< FIFO 1 Overrun Flag         */

/* Operating Mode Flags */
#define CAN_FLAG_WKU               ((uint32_t)0x31000008) /*!< Wake up Flag */
#define CAN_FLAG_SLAK              ((uint32_t)0x31000012) /*!< Sleep acknowledge Flag */
/* Note: When SLAK intterupt is disabled (SLKIE=0), no polling on SLAKI is possible. 
         In this case the SLAK bit can be polled.*/

/* Error Flags */
#define CAN_FLAG_EWG               ((uint32_t)0x10F00001) /*!< Error Warning Flag   */
#define CAN_FLAG_EPV               ((uint32_t)0x10F00002) /*!< Error Passive Flag   */
#define CAN_FLAG_BOF               ((uint32_t)0x10F00004) /*!< Bus-Off Flag         */
#define CAN_FLAG_LEC               ((uint32_t)0x30F00070) /*!< Last error code Flag */

  
#define CAN_IT_TME                  ((uint32_t)0x00000001) /*!< Transmit mailbox empty Interrupt*/

/* Receive Interrupts */
#define CAN_IT_FMP0                 ((uint32_t)0x00000002) /*!< FIFO 0 message pending Interrupt*/
#define CAN_IT_FF0                  ((uint32_t)0x00000004) /*!< FIFO 0 full Interrupt*/
#define CAN_IT_FOV0                 ((uint32_t)0x00000008) /*!< FIFO 0 overrun Interrupt*/
#define CAN_IT_FMP1                 ((uint32_t)0x00000010) /*!< FIFO 1 message pending Interrupt*/
#define CAN_IT_FF1                  ((uint32_t)0x00000020) /*!< FIFO 1 full Interrupt*/
#define CAN_IT_FOV1                 ((uint32_t)0x00000040) /*!< FIFO 1 overrun Interrupt*/

/* Operating Mode Interrupts */
#define CAN_IT_WKU                  ((uint32_t)0x00010000) /*!< Wake-up Interrupt*/
#define CAN_IT_SLK                  ((uint32_t)0x00020000) /*!< Sleep acknowledge Interrupt*/

/* Error Interrupts */
#define CAN_IT_EWG                  ((uint32_t)0x00000100) /*!< Error warning Interrupt*/
#define CAN_IT_EPV                  ((uint32_t)0x00000200) /*!< Error passive Interrupt*/
#define CAN_IT_BOF                  ((uint32_t)0x00000400) /*!< Bus-off Interrupt*/
#define CAN_IT_LEC                  ((uint32_t)0x00000800) /*!< Last error code Interrupt*/
#define CAN_IT_ERR                  ((uint32_t)0x00008000) /*!< Error Interrupt*/

/* Flags named as Interrupts : kept only for FW compatibility */
#define CAN_IT_RQCP0   CAN_IT_TME
#define CAN_IT_RQCP1   CAN_IT_TME
#define CAN_IT_RQCP2   CAN_IT_TME

#define CANINITFAILED               CAN_InitStatus_Failed
#define CANINITOK                   CAN_InitStatus_Success
#define CAN_FilterFIFO0             CAN_Filter_FIFO0
#define CAN_FilterFIFO1             CAN_Filter_FIFO1
#define CAN_ID_STD                  CAN_Id_Standard           
#define CAN_ID_EXT                  CAN_Id_Extended
#define CAN_RTR_DATA                CAN_RTR_Data         
#define CAN_RTR_REMOTE              CAN_RTR_Remote
#define CANTXFAILE                  CAN_TxStatus_Failed
#define CANTXOK                     CAN_TxStatus_Ok
#define CANTXPENDING                CAN_TxStatus_Pending
#define CAN_NO_MB                   CAN_TxStatus_NoMailBox
#define CANSLEEPFAILED              CAN_Sleep_Failed
#define CANSLEEPOK                  CAN_Sleep_Ok
#define CANWAKEUPFAILED             CAN_WakeUp_Failed        
#define CANWAKEUPOK                 CAN_WakeUp_Ok        


void CAN_DeInit(uint32_t CANx);

/* Initialization and Configuration functions *********************************/ 
uint8_t CAN_Init(uint32_t CANx, CAN_InitTypeDef* CAN_InitStruct);
void CAN_FilterInit(CAN_FilterInitTypeDef* CAN_FilterInitStruct);
void CAN_StructInit(CAN_InitTypeDef* CAN_InitStruct);
void CAN_SlaveStartBank(uint8_t CAN_BankNumber); 
void CAN_DBGFreeze(uint32_t CANx, FunctionalState NewState);
void CAN_TTComModeCmd(uint32_t CANx, FunctionalState NewState);

/* Transmit functions *********************************************************/
uint8_t CAN_Transmit(uint32_t CANx, CanTxMsg* TxMessage);
uint8_t CAN_TransmitStatus(uint32_t CANx, uint8_t TransmitMailbox);
void CAN_CancelTransmit(uint32_t CANx, uint8_t Mailbox);

/* Receive functions **********************************************************/
void CAN_Receive(uint32_t CANx, uint8_t FIFONumber, CanRxMsg* RxMessage);
void CAN_FIFORelease(uint32_t CANx, uint8_t FIFONumber);
uint8_t CAN_MessagePending(uint32_t CANx, uint8_t FIFONumber);


/* Operation modes functions **************************************************/
uint8_t CAN_OperatingModeRequest(uint32_t CANx, uint8_t CAN_OperatingMode);
uint8_t CAN_Sleep(uint32_t CANx);
uint8_t CAN_WakeUp(uint32_t CANx);

/* Error management functions *************************************************/
uint8_t CAN_GetLastErrorCode(uint32_t CANx);
uint8_t CAN_GetReceiveErrorCounter(uint32_t CANx);
uint8_t CAN_GetLSBTransmitErrorCounter(uint32_t CANx);

/* Interrupts and flags management functions **********************************/
void CAN_ITConfig(uint32_t CANx, uint32_t CAN_IT, FunctionalState NewState);
FlagStatus CAN_GetFlagStatus(uint32_t CANx, uint32_t CAN_FLAG);
void CAN_ClearFlag(uint32_t CANx, uint32_t CAN_FLAG);
ITStatus CAN_GetITStatus(uint32_t CANx, uint32_t CAN_IT);
void CAN_ClearITPendingBit(uint32_t CANx, uint32_t CAN_IT);

#ifdef __cplusplus
}
#endif

#endif 

/****END OF FILE****/
