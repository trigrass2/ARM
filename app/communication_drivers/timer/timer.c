/*
 * timer.c
 *
 *  Created on: 17/07/2015
 *      Author: joao.rosa
 */

#include "inc/hw_sysctl.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "driverlib/gpio.h"

#include <stdint.h>

#include "../adcp/adcp.h"

#include "../system_task/system_task.h"

//#include "set_pinout_udc_v2.0.h"
#include "../board_drivers/hardware_def.h"

#include "../i2c_onboard/exio.h"

uint16_t time = 0x00;
uint8_t  iib_sample = 0x00;

// Trata a interrup��o do timer0 a cada 1ms
void
GlobalTimerIntHandler(void)
{
	time++;
	iib_sample++;
	// Apaga a interrup��o do timer 0 A
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	GPIOPinWrite(DEBUG_BASE, DEBUG_PIN, ON);

	AdcpRead();
	//TaskSetNew(SAMPLE_ADCP);

	if(iib_sample >= 40)
	{
		iib_sample = 0;
		//TaskSetNew(0x10);
	}

	if(time >= 1000)
	{
		time = 0;
		TaskSetNew(SAMPLE_RTC);
		TaskSetNew(POWER_TEMP_SAMPLE);
		TaskSetNew(EEPROM_WRITE_REQUEST_CHECK);
		#if HARDWARE_VERSION == 0x21
		    TaskSetNew(LED_STATUS);
			#endif
	}



}


// O TIMER0 A � respons�vel pelo tempo de incremento de valor da rotina de teste
void
GlobalTimerInit(void)
{
    // Configura o TIMER0 com 32bits para a rotina de teste
    TimerConfigure(TIMER0_BASE, TIMER_CFG_32_BIT_PER);

    // Configura o TIMER 0 A com interrup��o de 1ms (divide o clock por 1000)
	TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet(SYSTEM_CLOCK_SPEED) / 1024 );


	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntRegister(INT_TIMER0A, GlobalTimerIntHandler);

	IntPrioritySet(INT_TIMER0A, 3);

	IntEnable(INT_TIMER0A);

	TimerEnable(TIMER0_BASE, TIMER_A);
}
