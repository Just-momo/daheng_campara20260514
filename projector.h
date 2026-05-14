#pragma once

#include "dlpc_common.h"
#include "dlpc34xx.h"
#include "dlpc347x_internal_patterns.h"
#include "cypress_i2c.h"
#include "math.h"
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "time.h"

#define MAX_WIDTH                         DLP3010_WIDTH
#define MAX_HEIGHT                        DLP3010_HEIGHT

#define FLASH_WRITE_BLOCK_SIZE            1024
#define FLASH_READ_BLOCK_SIZE             256

#define MAX_WRITE_CMD_PAYLOAD             (FLASH_WRITE_BLOCK_SIZE + 8)
#define MAX_READ_CMD_PAYLOAD              (FLASH_READ_BLOCK_SIZE  + 8)

static uint8_t                                   s_WriteBuffer[MAX_WRITE_CMD_PAYLOAD];
static uint8_t                                   s_ReadBuffer[MAX_READ_CMD_PAYLOAD];


class Projector
{
public:

	static uint32_t WriteI2C(uint16_t             WriteDataLength,
		uint8_t* WriteData,
		DLPC_COMMON_CommandProtocolData_s* ProtocolData);

	static uint32_t ReadI2C(uint16_t              WriteDataLength,
		uint8_t* WriteData,
		uint16_t                           ReadDataLength,
		uint8_t* ReadData,
		DLPC_COMMON_CommandProtocolData_s* ProtocolData);

	static void InitConnectionAndCommandLayer();
	static void WaitForSeconds(uint32_t Seconds);
	static void LoadPatternOrderTableEntryfromFlash();
	static void LoadPatternOrderTableEntry(uint8_t PatternSetIndex);
	static int initProjector();
	static void closeProjector();
	static void ProjectorTriggerOnce();
	static void ProjectorTriggerTwice();

private:

};


