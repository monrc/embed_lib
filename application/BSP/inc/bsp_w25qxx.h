#ifndef BSP_W25QXX_H
#define BSP_W25QXX_H

#include <stdint.h>
#include "lib_port.h"


typedef union
{
	struct
	{
		uint8_t busy : 1; // erase/write in progress
		uint8_t wel : 1;  // write enable latch
		uint8_t bp0 : 1;  // block protect bit0 (non-volatile)
		uint8_t bp1 : 1;  // block protect bit1 (non-volatile)
		uint8_t bp2 : 1;  // block protect bit2 (non-volatile)
		uint8_t tb : 1;	  // top/bottom protect (non-volatile)
		uint8_t sec : 1;  // sector protect (non-volatile)
		uint8_t srp0 : 1; // status register protect 0 (non-volatile)
	};
	uint8_t value;
} W25QXXStatus1_t;


typedef union
{
	struct
	{
		uint8_t srp1 : 1; // status register protect 1 (non-volatile)
		uint8_t qe : 1;	  // quad enable (non-volatile)
		uint8_t res : 1;  // reserved
		uint8_t lb1 : 1;  // security register lock bit1 (non-volatile otp)
		uint8_t lb2 : 1;  // block protect bit2 (non-volatile)
		uint8_t lb3 : 1;  // top/bottom protect (non-volatile)
		uint8_t cmp : 1;  // complement protect (non-volatile)
		uint8_t sus : 1;  // suspend status
	};
	uint8_t value;
} W25QXXStatus2_t;




enum W25QXX_cmd
{
	W25QXX_WRITE_ENABLE,
	W25QXX_SR_WR_ENBALE,
	W25QXX_WRITE_DISABLE,
	W25QXX_READ_STATUS1,
	W25QXX_READ_STATUS2,
	W25QXX_WRITE_STATUS,
	W25QXX_PAGE_PROGRAM,
	W25QXX_SECTOR_ERASE,
	W25QXX_BLOCK_ERASE,
	W25QXX_BLOCK_ERASE_64K,
	W25QXX_CHIP_ERASE,
	W25QXX_ERASE_PROGRAM_SUSPEND,
	W25QXX_ERASE_PROGRAM_RESUME,
	W25QXX_POWER_DOWN,
	W25QXX_READ_DATA,
	W25QXX_FAST_READ,
	W25QXX_RELEASE_POWERDOWN_ID,
	W25QXX_MANUFACTURER_DEVICE_ID,
	W25QXX_JEDEC_ID,
	W25QXX_READ_UNIQUE_ID,
	W25QXX_READ_SFDP_REG,
	W25QXX_ERASE_SECURITY_REG,
	W25QXX_PROGRAM_SECURITY_REG,
	W25QXX_READ_SECURITY_REG,
	W25QXX_ENABLE_QPI,
	W25QXX_ENABLE_RESET,
	W25QXX_RESET
};

typedef struct
{
	uint8_t txSize;
	uint8_t rxSize;

	union
	{
		struct
		{
			uint8_t cmd;
			union
			{
				struct //	W25QXX_WRITE_STATUS
				{
					W25QXXStatus1_t txS1;
					W25QXXStatus2_t txS2;
				};

				struct // erase/write cmds
				{
					uint8_t addrHH;
					uint8_t addrHL;
					uint8_t addrLL;
					uint8_t data[2];
				};

				uint8_t dummyAB[3]; // RELEASE_POWERDOWN_ID

				struct
				{
					uint8_t dummy90[2];
					uint8_t res;
				};

				uint8_t dummy4B[4]; // READ_UNIQUE_ID

				struct
				{
					uint8_t res5A[2]; // READ_SFDP_REG
					uint8_t addr;
					uint8_t dummy5A;
				};
			};
		};
		uint8_t txBuff[10];
	};

	union
	{
		uint8_t rxStatus;

		uint8_t devID;

		struct
		{
			uint8_t manufacturer;
			uint8_t deviceID;
		};

		struct
		{
			uint8_t manufact;
			uint8_t memoryType;
			uint8_t capacity;
		};

		uint8_t UID[8];

		uint8_t SFDP;

		uint8_t rxBuff[10];
	};
} W25QXX_t;

typedef struct
{
	uint8_t type;
	uint32_t addr;
	uint8_t *buff;
	uint32_t size;
	TaskHandle_t task;
} FlashMessage_t;

#endif
