#include "boot.h"
#include "mcu_init.h"
#include "uart.h"


#define APP_START_ADDR			0x8003000

typedef  void (*pFunction)(void);
extern void    FLASH_PageErase(uint32_t PageAddress);

typedef struct
{
	uint32_t dwEraseAddr;
	uint16_t wHighAddr;
	uint8_t Data[255];
}Download_t;

typedef enum
{
	HEX_LEN,
	HEX_HIGH_ADDR,
	HEX_LOW_ADDR,
	HEX_TYPE,
	HEX_DATA1,
	HEX_DATA2,
	HEX_CHECKSUM,
}HexFileFormat_t;	

typedef enum
{
	TYPE_DATA,
	TYPE_END_FILE,
	TYPE_EXTEND_SEGMENT_ADDR,
	TYPE_START_SEGMENT_ADDR,
	TYPE_EXTEND_LINEAR_ADDR,
	TYPE_START_LINEAR_ADDR,
}HexType_t;


Download_t stHex;


/*********************************************************
* Name		: get_line_data
* Function	: ��ȡһ������
* Input		: None
* Output	: None
* Return	: 0 �ɹ�	1���ճ�ʱ	2У�����
*********************************************************/
uint8_t get_line_data(uint8_t *pData)
{
	uint8_t byCheckSum = 0;
	uint8_t byLen = 0, i;
	if (false == get_char(&pData[HEX_LEN], 2000))
	{
		return 1;
	}
	while(byLen < pData[HEX_LEN] + 4)
	{
		if (false == get_char(&pData[HEX_HIGH_ADDR + byLen++], 2000))
		{
			return 1;
		}
	}
	
	for (i = 0; i <= byLen; i++)
	{
		byCheckSum += pData[i];
	}
	
	if (byCheckSum)
	{
		return 2;
	}
	
	return 0;
}

/*********************************************************
* Name		: get_line_data
* Function	: ��ȡһ������
* Input		: None
* Output	: None
* Return	: true ��ʼ����		false ��ת��Ӧ�ó���
*********************************************************/
bool power_up_check(void)
{
	uint8_t byAck;
	int32_t ticks = HAL_GetTick();
	
	while(HAL_GetTick() - ticks < 1000)
	{
		put_char('U');
		if (get_char(&byAck, 100))
		{
			if ('B' == byAck)
			{
				put_char('R');
				return true;
			}
		}
	}
	return false;
}


uint16_t FLASH_ReadHalfWord(uint32_t address)
{
	return *(__IO uint16_t*)address; 
}


/*********************************************************
* Name		: write_flash
* Function	: ������д��FLASH
* Input		: None
* Output	: None
* Return	: true �ɹ� 	false ʧ��
*********************************************************/
bool write_flash(void)
{
	uint8_t i;
	uint32_t dwAddr = 0;
	uint16_t *pData;
	uint16_t wLowAddr;
	
	wLowAddr = ((uint16_t)stHex.Data[HEX_HIGH_ADDR] << 8) | (uint16_t)stHex.Data[HEX_LOW_ADDR];
	dwAddr = ((uint32_t)stHex.wHighAddr << 16) | (uint32_t)wLowAddr;
	if (dwAddr < APP_START_ADDR)
	{
		return false;
	}

	while (stHex.dwEraseAddr < dwAddr + stHex.Data[HEX_LEN])
	{
		FLASH_PageErase(stHex.dwEraseAddr);
		FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);    //�ȴ��ϴβ������
		CLEAR_BIT(FLASH->CR, FLASH_CR_PER);					//���CR�Ĵ�����PERλ���˲���Ӧ����FLASH_PageErase()����ɣ�
															//����HAL�����沢û������Ӧ����HAL��bug��
		stHex.dwEraseAddr += FLASH_PAGE_SIZE;
	}
	
	pData = (uint16_t *)&stHex.Data[HEX_DATA1];
	for (i = 0; i < stHex.Data[HEX_LEN]; i += 2)
	{
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, dwAddr + i, *pData);
		if (*pData != FLASH_ReadHalfWord(dwAddr + i))
		{
			return false;
		}
		pData++;
	}
	
	return true;
}

/*********************************************************
* Name		: update_process
* Function	: ��������
* Input		: None
* Output	: None
* Return	: None
*********************************************************/
void update_process(void)
{
	uint8_t result;
	bool state = true;
	uint8_t i = 0;
	if (false == power_up_check())
	{
		jump_to_app();
	}
	
	while(i++ < 2)
	{
		get_line_data(stHex.Data);
		put_string("S\n");
	}
	
	while(1)
	{
		if (get_line_data(stHex.Data))
		{
			__set_FAULTMASK(1);   //STM32���������λ  
			NVIC_SystemReset(); 
		}
		switch(stHex.Data[HEX_TYPE])
		{
			case TYPE_DATA:
			{
				state = write_flash();
				break;
			}
			case TYPE_END_FILE:
			{
				put_string("S\n");
				jump_to_app();
				break;
			}
			case TYPE_EXTEND_LINEAR_ADDR:
			{
				stHex.wHighAddr = ((uint16_t)stHex.Data[HEX_DATA1] << 8) | (uint16_t)stHex.Data[HEX_DATA2];
				break;
			}
			default:
				break;
		}
		
		put_string(state ? "S\n" :"F\n");
	}
}

/*********************************************************
* Name		: jump_to_app
* Function	: ��ת��Ӧ�ó���
* Input		: None
* Output	: None
* Return	: None 
*********************************************************/
void jump_to_app(void)
{
	uint32_t JumpAddress;
	pFunction Jump_To_Application;
	
	
	if (((*(__IO uint32_t*)APP_START_ADDR) & 0x2FFE0000 ) == 0x20000000)
	{
		put_string("jump to app...\r\n");
		HAL_FLASH_Lock();
		HAL_UART_DeInit(&huart1);
		
		JumpAddress = *(__IO uint32_t*) (APP_START_ADDR + 4);
		/* Jump to user application */
		Jump_To_Application = (pFunction) JumpAddress;
		/* Initialize user application's Stack Pointer */
		__set_MSP(*(__IO uint32_t*) APP_START_ADDR);
		Jump_To_Application();	
	}
	else
	{
		put_string("app is empty reboot! \r\n");
		__set_FAULTMASK(1);   //STM32���������λ  
		NVIC_SystemReset();
	}
}



/*********************************************************
* Name		: bsp_init
* Function	: MCU Ӳ����ʼ��
* Input		: None
* Output	: None
* Return	: None 
*********************************************************/
void bsp_init(void)
{
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	//MX_IWDG_Init();
	MX_USART1_UART_Init();	//�رմ��ڽ����ж�ʹ��
	MX_USART2_UART_Init();
//	MX_TIM6_Init();
	HAL_FLASH_Unlock();
	
	memset(&stHex, 0, sizeof(stHex));
	stHex.dwEraseAddr = APP_START_ADDR;
}











