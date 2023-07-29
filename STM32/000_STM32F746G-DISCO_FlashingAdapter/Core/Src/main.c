/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "lwip.h"
#include "quadspi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tcpServerRAW.h"
#include "tcpClientRAW.h"
#include "tftpserver.h"
#include "flash.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef enum
{
	ext_sess = 0,
	sec_access_B,
	set_progmd,
	prog_sess,
	sec_access_A,
	idwrite_repair,
	idwrite_prog,
	erase_app,
	tester_present,
	req_transfer,
	transfer_app,
	exit_transfer,
	read_dtc,
	rid_check,
	ecu_reset,
	max_step
} sequence_step;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan1;
CAN_HandleTypeDef hcan2;

/* USER CODE BEGIN PV */
struct netif gnetif;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void Sequence_UDS_request(void);
static void Sequence_UDS_respone(uint8_t *data);
static void Calculate_Key(uint8_t level);
static void Control_Frame(void);
static void delay(void);
static void flash_test(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
sequence_step step = 0;
sequence_step wait_resp;
uint8_t readBuf[4096];
uint16_t counter2 = 0;

CAN_TxHeaderTypeDef TxHeader;
CAN_RxHeaderTypeDef RxHeader;

uint8_t count = 0 ;
uint8_t TxData[8];
uint8_t RxData[8];

uint8_t Rx_Buffer[8];
uint8_t Tx_Buffer[8];
uint8_t Manage_Data[8];

uint8_t sha256[32];
char Secret_K_B[] = {0x01,0x03,0x05,0x07,0x09,0x0B,0x0D,0x0F,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E};
char Secret_K_A[] = {0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E,0x01,0x03,0x05,0x07,0x09,0x0B,0x0D,0x0F};
char Seed[33] = {0x11,0xB4,0xFD,0xD5,0xC4,0x7B,0x9B,0xC3,0x39,0x54,0x2D,0x0B,0x65,0x46,0xED,0xC7,0x64,0xD1,0xD7,0xCF,0xCB,0x2D,0x7A,0xA6,0x2E,0xD6,0xCD,0xF2,0x43,0x70,0x8C,0x5A,0x5E};
uint8_t key[32];
Seed_Key seed_info;

uint32_t TxMailBox;

uint8_t Cur_Sts = 0;
uint8_t Cur_Ser = 0;
extern uint8_t isTFTPTransferCompleted;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_LWIP_Init();
  MX_TIM2_Init();
  MX_QUADSPI_Init();
  rb_CAN1_Init();
  /* USER CODE BEGIN 2 */
//  HAL_TIM_Base_Start_IT(&htim2);

  /* Initialize the QSPI */
  if (CSP_QUADSPI_Init() != HAL_OK)
  {
    Error_Handler();
  }

  // Initialize the TCP
//  tcp_client_init();
  tcp_server_init();
  // Initialize the TFTP server
  tftpd_init();

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  Msg_Info Rx_1 = {0x761, CAN_ID_STD, 8};
  Msg_Info Rx_2 = {0x03, CAN_ID_STD, 8};
  rb_CAN1_Filter_Config(Rx_1, 1, 0);
  rb_CAN1_Filter_Config(Rx_2, 2, 0);
  rb_CAN1_Start();
  rb_CAN1_IRQ_Config(CAN_IT_RX_FIFO0_MSG_PENDING);
  //Msg_Info Tx_1 = {0x04, CAN_ID_STD, 8};

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
    ethernetif_input(&gnetif);
    sys_check_timeouts();
    rb_resetUDPPCB();
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
    if(isTFTPTransferCompleted == 1)
    {
      isTFTPTransferCompleted = 0;
      //HAL_Delay(100);
      Sequence_UDS_request();
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void CAN1_RxCallBack(void)
{
	Msg_Info message_rx;
	rb_CAN1_Receive(&message_rx, 0, Rx_Buffer);
	Manage_Data[1]++;
	if(message_rx.ID == 0x03)
	{
		Sequence_UDS_request();
	  //flash_test();
	}
	else
	{
		if(Rx_Buffer[1] != 0x7F)
		{
			Sequence_UDS_respone(Rx_Buffer);
		}
		else
		{
		  if ((Rx_Buffer[2] != Cur_Ser) || (Rx_Buffer[3] != 0x78))
		  {
		    step = 0;
		  }
		}
	}
}

void flash_test(void)
{
  int64_t app_size_remain = 917376;
  int64_t block_size_remain;
  uint32_t app_size;
  uint32_t pointer_app = 4;
  uint32_t pointer_block = 0;
  uint8_t frame_num = 0x21;
  uint8_t step = 0;
  uint8_t block = 1;
  Msg_Info Tx_1 = {0x04, CAN_ID_STD, 8};
  uint8_t data[8];
  uint8_t last_data[8] = {0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};
  if (CSP_QSPI_Read(&app_size, BIN_SIZE_START_ADDR, BIN_SIZE_LEN) != HAL_OK)
  {
	  Error_Handler();
  }
  else
  {
	  app_size_remain = 917376;
	  while(app_size_remain > 0)
	  {
		if(app_size_remain > 4080)
		{
		  if (CSP_QSPI_Read(readBuf, pointer_app, 4080) != HAL_OK)
		  {
		   Error_Handler();
		   break;
		  }
		  block_size_remain = 4080;
		}
		else
		{
		  if (CSP_QSPI_Read(readBuf, pointer_app, app_size_remain) != HAL_OK)
		  {
		   Error_Handler();
		   break;
		  }
		  block_size_remain = app_size_remain;
		}
		frame_num = 0x21;
		pointer_block = 0;
		step = 0;
		while(block_size_remain > 0)
		{
			if(block_size_remain > 7)
			{
			  data[0] = frame_num++;
			  memcpy(&data[1], &readBuf[pointer_block], 7);
			}
			else
			{
			  memcpy(data, last_data, 8);
			  data[0] = frame_num++;
			  memcpy(&data[1], &readBuf[pointer_block], block_size_remain);
			}
			pointer_block += 7;
			if(frame_num >= 0x30)
			{
			  frame_num =  0x20;
			}
			block_size_remain -= 7;
			rb_CAN1_Transmit(Tx_1, data);
			delay();
		}
	  }
  }
}

void Sequence_UDS_request(void)
{
	uint8_t i;
	//static sequence_step step = 0;
	static uint8_t step_sec = 0; //request_seed
	static uint8_t block = 0; //request_seed
	static uint8_t last_block = 0; //request_seed
	static uint8_t wait_control_frame = 0; //wait control frame
	static int64_t app_size_remain = 917376;
	static int64_t block_size_remain;
	static uint32_t pointer_app = 4;
	Msg_Info UDS_req = {0x751, CAN_ID_STD, 8};
	wait_resp = step;
	switch (step)
	{
		case ext_sess:
		{
			uint8_t data[8] = {0x02, 0x10, 0x03, 0, 0, 0, 0, 0};
			rb_CAN1_Transmit(UDS_req, data);
			step++;
			if(step == max_step)
			{
				step = 0;
			}
			Cur_Ser = 0x10;
			break;
		}
		case sec_access_B:
		{
			if(step_sec == 0) //request seed
			{
				uint8_t data[8] = {0x02, 0x27, 0x11, 0, 0, 0, 0, 0};
                wait_control_frame = 0;
				rb_CAN1_Transmit(UDS_req, data);
				Cur_Ser = 0x27;
				step_sec++;
			}
			else //send_key
			{
				if(wait_control_frame == 0) //send first frame
				{
					uint8_t data[8] = {0x10, 0x22, 0x27, 0x12, 0, 0, 0, 0};
					memcpy(&data[4], key, 4);
					rb_CAN1_Transmit(UDS_req, data);
					wait_control_frame++;
				}
				else
				{
					uint8_t data[8];
					for(i = 0; i < 4; i++)
					{
						data[0] = 0x21 + i;
						memcpy(&data[1], &key[4 + i*7], 7);
						rb_CAN1_Transmit(UDS_req, data);
						delay();
					}
					wait_control_frame = 0;
					step_sec = 0;
					step++;
					if(step == max_step)
					{
						step = 0;
					}
				}
			}
			break;
		}
		case set_progmd:
		{
			uint8_t data[8] = {0x05, 0x31, 0x01, 0xC0, 0x01, 0, 0, 0};
			rb_CAN1_Transmit(UDS_req, data);
			step++;
			if(step == max_step)
			{
				step = 0;
			}
			Cur_Ser = 0x31;
			break;
		}
		case prog_sess:
		{
		  uint8_t data[8] = {0x02, 0x10, 0x02, 0, 0, 0, 0, 0};
		  rb_CAN1_Transmit(UDS_req, data);
		  step++;
		  if(step == max_step)
		  {
			step = 0;
		  }
		  Cur_Ser = 0x10;
		  break;
		}
		case sec_access_A:
		{
		  if(step_sec == 0) //request seed
		  {
			uint8_t data[8] = {0x02, 0x27, 0x01, 0, 0, 0, 0, 0};
			wait_control_frame = 0;
			rb_CAN1_Transmit(UDS_req, data);
			step_sec++;
			Cur_Ser = 0x27;
		  }
		  else //send_key
		  {
			if(wait_control_frame == 0) //send first frame
			{
			  uint8_t data[8] = {0x10, 0x22, 0x27, 0x02, 0, 0, 0, 0};
			  memcpy(&data[4], key, 4);
			  rb_CAN1_Transmit(UDS_req, data);
			  wait_control_frame++;
			}
			else
			{
			  uint8_t data[8];
			  for(i = 0; i < 4; i++)
			  {
				data[0] = 0x21 + i;
				memcpy(&data[1], &key[4 + i*7], 7);
				rb_CAN1_Transmit(UDS_req, data);
				delay();
			  }
			  wait_control_frame = 0;
			  step_sec = 0;
			  step++;
			  if(step == max_step)
			  {
				step = 0;
			  }
			}
		  }
		  break;
		}
		case idwrite_repair:
		{
		  if(wait_control_frame == 0) //send first frame
		  {
			uint8_t data[8] = {0x10, 0x0D, 0x2E, 0xF1, 0x98, 0x11, 0x22, 0x33};
			rb_CAN1_Transmit(UDS_req, data);
			wait_control_frame ++;
			Cur_Ser = 0x2E;
		  }
		  else
		  {
			uint8_t data[8] = {0x21, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x11};
			rb_CAN1_Transmit(UDS_req, data);
			wait_control_frame = 0;
			step++;
			if(step == max_step)
			{
			step = 0;
			}
		  }
		  break;
		}
		case idwrite_prog:
		{
		  uint8_t data[8] = {0x07, 0x2E, 0xF1, 0x99, 0x01, 0x07, 0x13, 0x14};
		  rb_CAN1_Transmit(UDS_req, data);
		  step++;
		  if(step == max_step)
		  {
			step = 0;
		  }
		  Cur_Ser = 0x2E;
		  break;
		}
		case erase_app:
		{
		  uint8_t data[8] = {0x05, 0x31, 0x01, 0xFF, 0x00, 0x00, 0x55, 0x55};
		  rb_CAN1_Transmit(UDS_req, data);
		  step++;
		  if(step == max_step)
		  {
			step = 0;
		  }
		  Cur_Ser = 0x31;
		  break;
		}
		case tester_present:
		{
		  uint8_t data[8] = {0x02, 0x3E, 0x00, 0x55, 0x55, 0x55, 0x55, 0x55};
		  rb_CAN1_Transmit(UDS_req, data);
		  step++;
		  if(step == max_step)
		  {
			step = 0;
		  }
		  Cur_Ser = 0x3E;
		  break;
		}
		case req_transfer:
		{
		  if(wait_control_frame == 0) //send first frame
		  {
			uint8_t data[8] = {0x10, 0x0C, 0x34, 0x0F, 0x45, 0x00, 0x01, 0x02};
			rb_CAN1_Transmit(UDS_req, data);
			wait_control_frame ++;
			Cur_Ser = 0x34;
		  }
		  else
		  {
			uint8_t data[8] = {0x21, 0x00, 0x00, 0x00, 0x0D, 0xFF, 0x80, 0x55};
			rb_CAN1_Transmit(UDS_req, data);
			wait_control_frame = 0;
			step++;
			if(step == max_step)
			{
			step = 0;
			}
		  }
		  pointer_app = 4;
		  app_size_remain = 917376;
		  last_block = 0;
		  block = 0x01;
		  break;
		}
		case transfer_app:
		{
			if(wait_control_frame == 0)
			{
				Cur_Ser = 0x36;
				wait_control_frame++;
				if(app_size_remain > 4080)
				{
				  if (CSP_QSPI_Read(readBuf, pointer_app, 4080) != HAL_OK)
				  {
				   Error_Handler();
				   break;
				  }
				  block_size_remain = 4080;
				}
				else
				{
				  if (CSP_QSPI_Read(readBuf, pointer_app, app_size_remain) != HAL_OK)
				  {
				   Error_Handler();
				   break;
				  }
				  block_size_remain = app_size_remain;
				  last_block = 1;
				}
				uint8_t data[8] = {0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};
				data[0] = ((((uint32_t)block_size_remain) & 0x0F00u) | 0x1000u) >> 8;
				data[1] = (((uint32_t)block_size_remain) & 0x00FFu) + 2;
				data[2] = 0x36;
				data[3] = block++;
				memcpy(&data[4], readBuf, 4);
				rb_CAN1_Transmit(UDS_req, data);
				block_size_remain -= 4;
				app_size_remain -= 4080;
				pointer_app += 4080;
			}
			else
			{
				uint8_t frame_num = 0x21;
				uint16_t pointer_block = 4;
				while(block_size_remain > 0)
				{
					uint8_t data[8] = {0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};
					if(block_size_remain > 7)
					{
					  data[0] = frame_num++;
					  memcpy(&data[1], &readBuf[pointer_block], 7);
					}
					else
					{
					  data[0] = frame_num++;
					  memcpy(&data[1], &readBuf[pointer_block], block_size_remain);
					}
					pointer_block += 7;
					if(frame_num >= 0x30)
					{
					  frame_num =  0x20;
					}
					block_size_remain -= 7;
					rb_CAN1_Transmit(UDS_req, data);
					delay();
				}
				wait_control_frame = 0;
				if (last_block == 1)
				{
					step++;
					if(step == max_step)
					{
					step = 0;
					}
				}
			}
			break;
		}
		case exit_transfer:
		{
			Cur_Ser = 0x37;
			uint8_t data[8] = {0x01, 0x37, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};
			rb_CAN1_Transmit(UDS_req, data);
			step++;
			if(step == max_step)
			{
			step = 0;
			}
			break;
		}
		case read_dtc:
		{
			Cur_Ser = 0x19;
			uint8_t data[8] = {0x03, 0x19, 0x02, 0x4B, 0x55, 0x55, 0x55, 0x55};
			rb_CAN1_Transmit(UDS_req, data);
			step++;
			if(step == max_step)
			{
			step = 0;
			}
			break;
		}
		case rid_check:
		{
			Cur_Ser = 0x31;
			uint8_t data[8] = {0x05, 0x31, 0x01, 0xFF, 0x01, 0x00, 0x55, 0x55};
			rb_CAN1_Transmit(UDS_req, data);
			step++;
			if(step == max_step)
			{
			step = 0;
			}
			break;
		}
		case ecu_reset:
		{
			Cur_Ser = 0x11;
			uint8_t data[8] = {0x02, 0x11, 0x01, 0x55, 0x55, 0x55, 0x55, 0x55};
			rb_CAN1_Transmit(UDS_req, data);
			step++;
			if(step == max_step)
			{
			step = 0;
			}
			break;
		}
		default:
		{

		}
	}
}

static void Sequence_UDS_respone(uint8_t *data)
{
	switch (wait_resp)
	{
		case ext_sess:
		{
			if((data[1] == 0x50) && (data[2] == 0x03))
			{
				Sequence_UDS_request();
			}
			break;
		}
		case sec_access_B:
		{
			if((data[0] == 0x10) && (data[1] == 0x22) && (data[2] == 0x67))
			{
				memcpy(Seed, &data[3], 5);
				Control_Frame();
			}
			else if((data[0] >= 0x21) && (data[0] <= 0x24))
			{
				memcpy(&Seed[5 + (data[0] - 0x21) * 7], &data[1], 7);
				if(data[0] == 0x24)
				{
					Calculate_Key(11);
					Sequence_UDS_request();
				}
			}
			else if (data[0] == 0x30) //control_frame
			{
				Sequence_UDS_request();
			}
			else if ((data[1] == 0x67) && (data[2] == 0x12))
			{
				Sequence_UDS_request();
			}
			else
			{
			  step = 0;
			}
			break;
		}
		case set_progmd:
		{
		  if((data[1] == 0x71) && (data[2] == 0x01) && (data[3] == 0xC0) && (data[4] == 0x01))
		  {
			Sequence_UDS_request();
		  }
		  break;
		}
		case prog_sess:
		{
		  if((data[1] == 0x50) && (data[2] == 0x02))
		  {
			Sequence_UDS_request();
		  }
		  break;
		}
		case sec_access_A:
		{
		  if((data[0] == 0x10) && (data[1] == 0x22) && (data[2] == 0x67))
		  {
			memcpy(Seed, &data[3], 5);
			Control_Frame();
		  }
		  else if((data[0] >= 0x21) && (data[0] <= 0x24))
		  {
			memcpy(&Seed[5 + (data[0] - 0x21) * 7], &data[1], 7);
			if(data[0] == 0x24)
			{
			  Calculate_Key(1);
			  Sequence_UDS_request();
			}
		  }
		  else if (data[0] == 0x30) //control_frame
		  {
			Sequence_UDS_request();
		  }
		  else if ((data[1] == 0x67) && (data[2] == 0x02))
		  {
			Sequence_UDS_request();
		  }
		  else
		  {
			step = 0;
		  }
		  break;
		}
		case idwrite_repair:
		{
		  if (data[0] == 0x30) //control_frame
		  {
		    Sequence_UDS_request();
		  }
		  else if ((data[1] == 0x6E) && (data[2] == 0xF1) && (data[3] == 0x98))
		  {
		    Sequence_UDS_request();
		  }
		  else
		  {
			  step = 0;
		  }
		  break;
		}
		case idwrite_prog:
		{
		  if ((data[1] == 0x6E) && (data[2] == 0xF1) && (data[3] == 0x99))
		  {
		    Sequence_UDS_request();
		  }
		  else
		  {
			  step = 0;
		  }
		  break;
		}
		case erase_app:
		{
		  if ((data[1] == 0x71) && (data[2] == 0x01) && (data[3] == 0xFF))
		  {
			Sequence_UDS_request();
		  }
		  else
		  {
			  step = 0;
		  }
		  break;
		}
		case tester_present:
		{
		  if ((data[1] == 0x7E) && (data[2] == 0x00))
		  {
			Sequence_UDS_request();
		  }
		  else
		  {
			  step = 0;
		  }
		  break;
		}
		case req_transfer:
		{
		  if (data[0] == 0x30) //control_frame
		  {
			Sequence_UDS_request();
		  }
		  else if ((data[1] == 0x74) && (data[2] == 0x20) && (data[3] == 0x0F) && (data[4] == 0xF2))
		  {
			Sequence_UDS_request();
		  }
		  else
		  {
			step = 0;
		  }
		  break;
		}
		case transfer_app:
		{
			if (data[0] == 0x30) //control_frame
			{
				Sequence_UDS_request();
			}
			else if ((data[1] == 0x76))
			{
				Sequence_UDS_request();
			}
			else
			{
				step = 0;
			}
			break;
		}
		case exit_transfer:
		{
			if (data[1] == 0x77)
			{
				Sequence_UDS_request();
			}
			else
			{
				step = 0;
			}
			break;
		}
		case read_dtc:
		{
			if ((data[1] == 0x59) && (data[2] == 0x02) && (data[3] == 0x4B))
			{
				Sequence_UDS_request();
			}
			else
			{
				step = 0;
			}
			break;
		}
		case rid_check:
		{
			if ((data[1] == 0x71) && (data[2] == 0x01) && (data[3] == 0xFF))
			{
				Sequence_UDS_request();
			}
			else
			{
				step = 0;
			}
			break;
		}
		case ecu_reset:
		{
			if ((data[1] == 0x51) && (data[2] == 0x01))
			{
				step = 0;
			}
			else
			{
				step = 0;
			}
			break;
		}
		default:
		{

		}
	}
}

static void Control_Frame(void)
{
	Msg_Info tx_msg = {0x751, CAN_ID_STD, 8};
	uint8_t data[8] = {0x30, 0, 0, 0, 0, 0, 0, 0};
	rb_CAN1_Transmit(tx_msg, data);
}

static void Calculate_Key(uint8_t level)
{
	if(level == 11) // level B
	{
    memcpy(seed_info.secret_key, Secret_K_B, sizeof(Secret_K_B));
    seed_info.secret_key_length = sizeof(Secret_K_B);
	}
	else
	{
	  memcpy(seed_info.secret_key, Secret_K_A, sizeof(Secret_K_A));
	  seed_info.secret_key_length = sizeof(Secret_K_A);
	}
	memcpy(seed_info.seed, Seed, sizeof(Seed));
	seed_info.seed_length = sizeof(Seed);
	Cal_Key(seed_info, key);
}

static void delay(void)
{
	uint64_t index;
	for (index = 0; index < 10000; index++)
	{

	}

}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  char buf[100];
  sprintf (buf, "Sending TCPServer Message %d\n", counter2);
  rb_TCPServerSend(buf);
  HAL_GPIO_TogglePin(GPIOI, GPIO_PIN_1);

  counter2++;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
