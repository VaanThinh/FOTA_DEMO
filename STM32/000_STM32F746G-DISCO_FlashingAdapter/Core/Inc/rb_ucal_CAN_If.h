#ifndef _RB_UCAL_CAN_If_H_
#define _RB_UCAL_CAN_If_H_

#include "stm32f7xx_hal.h"
#include "main.h"

#define TRUE      1
#define FALSE     0

typedef struct Message_Info
{
    uint32_t    ID;
    uint8_t       Type;
    uint8_t       DLC;
}Msg_Info;


/**
  * @brief  Initialize CAN1.
  * @retval void
  */
void rb_CAN1_Init(void);

/**
  * @brief  De-Initialize CAN1.
  * @retval void
  */
void rb_CAN1_DeInit(void);

/**
  * @brief  Receive message on CAN1
  * @param  ID For
  * @param  Msg_Info store message info (ID, DLC, Standard or Extend)
  * @param  msg store message data
  * @retval void
  */
void rb_CAN1_Receive(Msg_Info* Message, uint8_t FIFO_Num, uint8_t* msg);

/**
  * @brief  Transmit message on CAN1
  * @param  ID For
  * @param  Msg_Info store message info (ID, DLC, Standard or Extend)
  * @param  msg store message data
  * @retval void
  */
void rb_CAN1_Transmit(Msg_Info Message, uint8_t* msg);

/**
  * @brief  Enable interrupts for CAN1.
  * @param  ActiveITs indicates which interrupts will be enabled.
  *         This parameter can be any combination of @arg CAN_Interrupts.
  * @retval void
  */
void rb_CAN1_IRQ_Config(uint32_t ActiveITs);

/**
  * @brief  Config Filter for CAN1.
  * @param  ActiveITs indicates which interrupts will be enabled.
  *         This parameter can be any combination of @arg CAN_Interrupts.
  * @retval void
  */
void rb_CAN1_Filter_Config(Msg_Info Message, uint8_t Filternumber, uint8_t FIFO_num);

/**
  * @brief  CAN1 start receive, send message.
  * @param  ActiveITs indicates which interrupts will be enabled.
  *         This parameter can be any combination of @arg CAN_Interrupts.
  * @retval void
  */
void rb_CAN1_Start(void);

#endif
