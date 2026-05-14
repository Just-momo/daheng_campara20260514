#include "projector.h"

/**
 * Implement the I2C write transaction here. The sample code here sends
 * data to the controller via the Cypress USB-Serial adapter.
 */
uint32_t Projector::WriteI2C(uint16_t             WriteDataLength,
	uint8_t* WriteData,
	DLPC_COMMON_CommandProtocolData_s* ProtocolData)
{
	bool Status = true;
	//printf("Write I2C Starts, length %d!!! \n", WriteDataLength);
	Status = CYPRESS_I2C_WriteI2C(WriteDataLength, WriteData);
	if (Status != true)
	{
		//printf("Write I2C Error!!! \n");
		return FAIL;
	}

	return SUCCESS;
}

/**
 * Implement the I2C write/read transaction here. The sample code here
 * receives data from the controller via the Cypress USB-Serial adapter.
 */
uint32_t Projector::ReadI2C(uint16_t              WriteDataLength,
	uint8_t* WriteData,
	uint16_t                           ReadDataLength,
	uint8_t* ReadData,
	DLPC_COMMON_CommandProtocolData_s* ProtocolData)
{
	bool Status = 0;
	//printf("Write/Read I2C Starts, length %d!!! \n", WriteDataLength);
	Status = CYPRESS_I2C_WriteI2C(WriteDataLength, WriteData);
	if (Status != true)
	{
		//printf("Write I2C Error!!! \n");
		return FAIL;
	}

	Status = CYPRESS_I2C_ReadI2C(ReadDataLength, ReadData);
	if (Status != true)
	{
		//printf("Read I2C Error!!! \n");
		return FAIL;
	}

	return SUCCESS;
}

/**
 * Initialize the command layer by setting up the read/write buffers and
 * callbacks.
 */
void Projector::InitConnectionAndCommandLayer()
{
	DLPC_COMMON_InitCommandLibrary(s_WriteBuffer,
		sizeof(s_WriteBuffer),
		s_ReadBuffer,
		sizeof(s_ReadBuffer),
		WriteI2C,
		ReadI2C);

	CYPRESS_I2C_ConnectToCyI2C();
}

void Projector::WaitForSeconds(uint32_t Seconds)
{
	uint32_t retTime = (uint32_t)(time(0)) + Seconds;	// Get finishing time.
	while (time(0) < retTime);					        // Loop until it arrives.
}


void Projector::LoadPatternOrderTableEntryfromFlash()
{
	DLPC34XX_PatternOrderTableEntry_s PatternOrderTableEntry;

	/* Reload from Flash */
	DLPC34XX_WritePatternOrderTableEntry(DLPC34XX_WC_RELOAD_FROM_FLASH, &PatternOrderTableEntry);
}

void Projector::LoadPatternOrderTableEntry(uint8_t PatternSetIndex)
{
	DLPC34XX_PatternOrderTableEntry_s PatternOrderTableEntry;

	/* Set PatternOrderTableEntry to select specific Pattern Set and configure settings */
	PatternOrderTableEntry.PatSetIndex = 1;
	PatternOrderTableEntry.NumberOfPatternsToDisplay = PatternSetIndex;
	PatternOrderTableEntry.RedIlluminator = DLPC34XX_IE_DISABLE;
	PatternOrderTableEntry.GreenIlluminator = DLPC34XX_IE_DISABLE;
	PatternOrderTableEntry.BlueIlluminator = DLPC34XX_IE_ENABLE;
	PatternOrderTableEntry.PatternInvertLsword = 0;
	PatternOrderTableEntry.PatternInvertMsword = 0;
	PatternOrderTableEntry.IlluminationTime = 8000;
	PatternOrderTableEntry.PreIlluminationDarkTime = 6000;
	PatternOrderTableEntry.PostIlluminationDarkTime = 6000;
	DLPC34XX_WritePatternOrderTableEntry(DLPC34XX_WC_START, &PatternOrderTableEntry);
}


int Projector::initProjector()
{
	InitConnectionAndCommandLayer();
	WaitForSeconds(0.5);
	/* TI DLP Pico EVMs use a GPIO handshake scheme for the controller I2C bus
	 * arbitration. Call this method if using a TI EVM, remove otherwise
	 */
	bool Status = CYPRESS_I2C_RequestI2CBusAccess();
	WaitForSeconds(0.5);
	if (Status != true)
	{
		//printf("Error Request I2C Bus ACCESS!!!");
		return 1;
	}

	DLPC34XX_ControllerDeviceId_e DeviceId = DLPC34XX_CDI_DLPC3478;//一定注意要跟连接的投影仪控制器型号对应
	DLPC34XX_ReadControllerDeviceId(&DeviceId);
	//printf("Controller Devicde Id = %d \n", DeviceId);
	WaitForSeconds(0.5);
	if (Status == true)//投射所有的内部图案
	{
		/* Stop pattern display */
		DLPC34XX_WriteInternalPatternControl(DLPC34XX_PC_STOP, 0);
		/* Load Pattern Order Table Entry from Flash */
		LoadPatternOrderTableEntryfromFlash();
		WaitForSeconds(0.2);
		//内部图案模式
		DLPC34XX_WriteOperatingModeSelect(DLPC34XX_OM_SENS_INTERNAL_PATTERN);
		WaitForSeconds(0.2);

		//触发一次
		DLPC34XX_WriteInternalPatternControl(DLPC34XX_PC_START, 0x00);
	}
	//if (Status == true)//投射指定的内部图案
	//{
	//	DLPC34XX_PatternOrderTableEntry_s PatternOrderTableEntry;
	//	// Set the operating mode to internal pattern streaming.
	//	DLPC34XX_WriteOperatingModeSelect(DLPC34XX_OM_SENS_INTERNAL_PATTERN);
	//	WaitForSeconds(1);
	//	DLPC34XX_WriteInternalPatternControl(DLPC34XX_PC_RESET, 0x00); 
	//	WaitForSeconds(1);
	//	PatternOrderTableEntry.PatSetIndex = 1;
	//	PatternOrderTableEntry.NumberOfPatternsToDisplay = 4;
	//	PatternOrderTableEntry.RedIlluminator = DLPC34XX_IE_DISABLE;
	//	PatternOrderTableEntry.GreenIlluminator = DLPC34XX_IE_DISABLE;
	//	PatternOrderTableEntry.BlueIlluminator = DLPC34XX_IE_ENABLE;
	//	PatternOrderTableEntry.PatternInvertLsword = 0;
	//	PatternOrderTableEntry.PatternInvertMsword = 0;
	//	PatternOrderTableEntry.IlluminationTime = 8000;
	//	PatternOrderTableEntry.PreIlluminationDarkTime = 6000;
	//	PatternOrderTableEntry.PostIlluminationDarkTime = 6000;
	//	DLPC34XX_WritePatternOrderTableEntry(DLPC34XX_WC_START, &PatternOrderTableEntry);
	//	DLPC34XX_WriteInternalPatternControl(DLPC34XX_PC_START, 0x00);
	//}
	return 0;
}

void Projector::ProjectorTriggerOnce()
{
	//设置Trigger Out2触发
	DLPC34XX_WriteTriggerOutConfiguration(DLPC34XX_TT_TRIGGER2, DLPC34XX_TE_ENABLE, DLPC34XX_TI_NOT_INVERTED, 50);
	//Trigger out 2; Enable; not Invert; Delay(us) 0或者指定时间;

	DLPC34XX_WriteInternalPatternControl(DLPC34XX_PC_START, 0x00);
}

void Projector::ProjectorTriggerTwice()
{
	//设置Trigger Out2触发
	DLPC34XX_WriteTriggerOutConfiguration(DLPC34XX_TT_TRIGGER2, DLPC34XX_TE_ENABLE, DLPC34XX_TI_NOT_INVERTED, 50);
	//Trigger out 2; Enable; not Invert; Delay(us) 0或者指定时间;

	DLPC34XX_WriteInternalPatternControl(DLPC34XX_PC_START, 0x01);
}

void Projector::closeProjector()
{
	DLPC34XX_WriteInternalPatternControl(DLPC34XX_PC_STOP, 0);
	CYPRESS_I2C_RelinquishI2CBusAccess();
}