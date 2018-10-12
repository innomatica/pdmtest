#include <stdarg.h>
#include <stdio.h>
#include "board.h"
#include "stm32l4xx_hal_tim.h"

extern DAC_HandleTypeDef hdac1;
extern TIM_HandleTypeDef htim6;
extern UART_HandleTypeDef huart2;
/** Formatted string output to USB_CDC
 *
 *	\param printf() like parameters
#include "usbd_cdc.h"
extern USBD_HandleTypeDef hUsbDeviceFS;
void USB_Printf(const char* format,...)
{
	char buffer[256];
	int length;
	va_list args;
	va_start(args, format);

	length = vsprintf(buffer, format, args);
	if(length)
		USBD_CDC_SetTxBuffer(&hUsbDeviceFS, (uint8_t*)buffer, length);

	va_end(args);
}
 */

// 1KHz sinewave at various sampling freq
#if (SAMPLE_FREQ == 24000)
static uint16_t Sin1K[] = {2048,2578,3072,3496,3821,4026,4095,4026,3821,
                           3496,3072,2578,2048,1517,1024,599,274,69,0,69,
                           274,599,1023,1517
                          };
#elif (SAMPLE_FREQ == 16000)
static uint16_t Sin1K[] = {2048,2831,3496,3940,4095,3940,3496,2831,2048,
                           1264,599, 155,0,155,599,1264
                          };
#endif


void Play_Sine(void)
{
	HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*)Sin1K,
	                  sizeof(Sin1K)/sizeof(Sin1K[0]), DAC_ALIGN_12B_R);
	HAL_TIM_Base_Start(&htim6);
}

void Stop_DAC(void)
{
	HAL_TIM_Base_Stop(&htim6);
	HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);
}


/** Formatted string ouput to UART
 *
 *	\param printf() like parameters
 */
void UART_Printf(const char* format,...)
{
	char buffer[256];
	int length;
	va_list args;
	va_start(args, format);

	length = vsprintf(buffer, format, args);
	if(length)
		HAL_UART_Transmit(&huart2, (uint8_t*)buffer, length, 1000);

	va_end(args);
}


/** Microsecond delay
 *
 * \warning This is subject to error since the delay relies on execution of
 *		nop().
 */
void USecDelay(unsigned usec)
{
	while(usec-- > 0)
	{
		// approximately 1 usec delay in 32MHz clock
		asm(
		    "nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t"
		    "nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t"
		    "nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t"
		);
	}
}

uint8_t PushButton_Read(void)
{
	return BTN_READ;
}

void SerialComm_Init(void)
{
	// clear USART interrupt
	HAL_NVIC_ClearPendingIRQ(USART1_IRQn);
	// enable RX interrupt
	USART1_ENABLE_RX_IT;
}

void SerialComm_SendByteArray(uint8_t *buffer, int size)
{
	// note that HAL_UART_TransmitIT no longer works
	HAL_UART_Transmit(&huart2, buffer, size, 1000);
}

#if UNIT_TEST

#endif
