#include "rb_ucal_CAN_If.h"

static CAN_HandleTypeDef hcan;
static uint8_t Init_Flag = FALSE;
static uint8_t FilCof_Flag;

/**
  * @brief  Initialize CAN1.
  * @retval void
  */
void rb_CAN1_Init(void)
{
	if(Init_Flag != TRUE)
	{
	  hcan.Instance = CAN1;
	  hcan.Init.Prescaler = 12;
	  hcan.Init.Mode = CAN_MODE_NORMAL;
	  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
	  hcan.Init.TimeSeg1 = CAN_BS1_2TQ;
	  hcan.Init.TimeSeg2 = CAN_BS2_6TQ;
	  hcan.Init.TimeTriggeredMode = DISABLE;
	  hcan.Init.AutoBusOff = DISABLE;
	  hcan.Init.AutoWakeUp = DISABLE;
	  hcan.Init.AutoRetransmission = DISABLE;
	  hcan.Init.ReceiveFifoLocked = DISABLE;
	  hcan.Init.TransmitFifoPriority = DISABLE;
	  Init_Flag = TRUE;
	  if (HAL_CAN_Init(&hcan) != HAL_OK)
	  {
	  	Error_Handler();
		Init_Flag = FALSE;
	  }
	}
}

/**
  * @brief  De-Initialize CAN1.
  * @retval void
  */
void rb_CAN1_DeInit(void)
{
	if(Init_Flag == TRUE)
	{
    HAL_CAN_DeInit(&hcan);
		Init_Flag = FALSE;
	}
}

/**
  * @brief  Receive message on CAN1
  * @param  ID For
  * @param  Msg_Info store message info (ID, DLC, Standard or Extend)
  * @param  msg store message data
  * @retval void
  */
void rb_CAN1_Receive(Msg_Info* Message, uint8_t FIFO_Num, uint8_t* msg)
{
	CAN_RxHeaderTypeDef RxHeader;
	if(Init_Flag == TRUE)
	{
		HAL_CAN_GetRxMessage(&hcan, FIFO_Num, &RxHeader, msg);

		Message->DLC = RxHeader.DLC;

		if(RxHeader.IDE == CAN_ID_STD)
		{
				Message->ID = RxHeader.StdId;
		}
		else
		{
				Message->ID = RxHeader.ExtId;
		}

	}
}

/**
  * @brief  Transmit message on CAN1
  * @param  ID For
  * @param  Msg_Info store message info (ID, DLC, Standard or Extend)
  * @param  msg store message data
  * @retval void
  */
void rb_CAN1_Transmit(Msg_Info Message, uint8_t* msg)
{
	uint32_t TxMailbox;
	CAN_TxHeaderTypeDef TxHeader;
	if(Init_Flag == TRUE)
	{
		TxHeader.RTR = CAN_RTR_DATA;
		TxHeader.DLC = Message.DLC;

		if(Message.Type == CAN_ID_STD)
		{
				TxHeader.IDE = CAN_ID_STD;
				TxHeader.StdId = Message.ID;
		}
		else
		{
				TxHeader.IDE = CAN_ID_EXT;
				TxHeader.ExtId = Message.ID;
		}

		HAL_CAN_AddTxMessage(&hcan, &TxHeader, msg, &TxMailbox);
	}
}

/**
  * @brief  Enable interrupts for CAN1.
  * @param  ActiveITs indicates which interrupts will be enabled.
  *         This parameter can be any combination of @arg CAN_Interrupts.
  * @retval void
  */
void rb_CAN1_IRQ_Config(uint32_t ActiveITs)
{
    if(Init_Flag == TRUE)
    {
        HAL_CAN_ActivateNotification(&hcan, ActiveITs);
    }
}

/**
  * @brief  Config Filter for CAN1.
  * @param  ActiveITs indicates which interrupts will be enabled.
  *         This parameter can be any combination of @arg CAN_Interrupts.
  * @retval void
  */
void rb_CAN1_Filter_Config(Msg_Info Message, uint8_t Filternumber, uint8_t FIFO_num)
{
	CAN_FilterTypeDef sFilter;
	uint32_t temp;

	if(Init_Flag == TRUE)
	{
		sFilter.FilterActivation = 1;
		/* default 20 for CAN1, 8 for CAN2*/
		sFilter.SlaveStartFilterBank = 20;

		sFilter.FilterFIFOAssignment = FIFO_num;

		if(Message.Type == CAN_ID_STD)
		{
			sFilter.FilterIdHigh = (Message.ID << 5);
			sFilter.FilterIdLow = 0;
		}
		else
		{
			temp = (Message.ID & 0x1FFFE000) >> 13;
			sFilter.FilterIdHigh = (uint16_t)temp;
			temp = ((Message.ID & 0x00001FFF) << 3) | (0x1<<2);
			sFilter.FilterIdLow = (uint16_t)temp;
		}

		sFilter.FilterMaskIdHigh = 0x0000;
		sFilter.FilterMaskIdLow = 0x0000;

		/* default IDLIST, 32BIT*/
		sFilter.FilterMode = ((uint8_t)0x01);
		sFilter.FilterScale = ((uint8_t)0x01);

		sFilter.FilterBank = Filternumber;

		HAL_CAN_ConfigFilter(&hcan, &sFilter);
		FilCof_Flag = TRUE;
	}
}

void rb_CAN1_Start(void)
{
	if((Init_Flag==TRUE) && (FilCof_Flag==TRUE))
	{
		HAL_CAN_Start(&hcan);
	}
}

/**
  * @brief  CAN1 FIFO1 Interrupt
  * @param  ActiveITs indicates which interrupts will be enabled.
  *         This parameter can be any combination of @arg CAN_Interrupts.
  * @retval void
  */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	CAN1_RxCallBack();
}

