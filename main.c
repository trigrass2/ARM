/******************************************************************************
 * Copyright (C) 2017 by LNLS - Brazilian Synchrotron Light Laboratory
 *
 * Redistribution, modification or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. LNLS and
 * the Brazilian Center for Research in Energy and Materials (CNPEM) are not
 * liable for any misuse of this material.
 *
 *****************************************************************************/

/**
 * @file main.c
 * @brief DRS Application.
 *
 * @author joao.rosa
 *
 * @date 14/04/2015
 *
 */

#include <communication_drivers/psmodules/fbp_dclink/fbp_dclink_main.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_gpio.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/cpu.h"
#include "driverlib/ram.h"
#include "driverlib/flash.h"
#include "driverlib/timer.h"
#include "driverlib/ipc.h"
#include "driverlib/usb.h"

#include "communication_drivers/signals_onboard/signals_onboard.h"
#include "communication_drivers/rs485/rs485.h"
#include "communication_drivers/rs485_bkp/rs485_bkp.h"
#include "communication_drivers/ihm/ihm.h"
#include "communication_drivers/ethernet/ethernet_uip.h"
#include "communication_drivers/can/can_bkp.h"
#include "communication_drivers/usb_device/superv_cmd.h"
#include "communication_drivers/i2c_onboard/i2c_onboard.h"
#include "communication_drivers/i2c_onboard/rtc.h"
#include "communication_drivers/i2c_onboard/eeprom.h"
#include "communication_drivers/i2c_onboard/exio.h"
#include "communication_drivers/adcp/adcp.h"
#include "communication_drivers/timer/timer.h"
#include "communication_drivers/system_task/system_task.h"
#include "communication_drivers/flash/flash_mem.h"
#include "communication_drivers/parameters/system/system.h"
#include "communication_drivers/ipc/ipc_lib.h"
#include "communication_drivers/bsmp/bsmp_lib.h"

#include "communication_drivers/psmodules/fbp/fbp_main.h"
#include "communication_drivers/psmodules/fbp_dclink/fbp_dclink_main.h"

#include "hardware_def.h"

extern unsigned long RamfuncsLoadStart;
extern unsigned long RamfuncsRunStart;
extern unsigned long RamfuncsLoadSize;


#define M3_MASTER 0
#define C28_MASTER 1

uint8_t read_rtc, read_rtc_status;

uint8_t read_add_rs485, set_add_rs485, read_add_IP, set_add_IP, read_display_sts, set_display_sts, read_isodcdc_sts, set_isodcdc_sts, read_adcp, read_flash_sn;
uint8_t add485, stsdisp, stsisodcdc;
uint32_t addIP;

int main(void) {
	
	volatile unsigned long ulLoop;

	// Disable Protection
	HWREG(SYSCTL_MWRALLOW) =  0xA5A5A5A5;

	// Tells M3 Core the vector table is at the beginning of C0 now.
	HWREG(NVIC_VTABLE) = 0x20005000;

	// Sets up PLL, M3 running at 75MHz and C28 running at 150MHz
	SysCtlClockConfigSet(SYSCTL_USE_PLL | (SYSCTL_SPLLIMULT_M & 0xF) |
	                         SYSCTL_SYSDIV_1 | SYSCTL_M3SSDIV_2 |
	                         SYSCTL_XCLKDIV_4);

// Copy time critical code and Flash setup code to RAM
// This includes the following functions:  InitFlash();
// The  RamfuncsLoadStart, RamfuncsLoadSize, and RamfuncsRunStart
// symbols are created by the linker. Refer to the device .cmd file.
    memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);

// Call Flash Initialization to setup flash waitstates
// This function must reside in RAM
    FlashInit();

    // Configure the board peripherals
    //HardwareInit();

    pinout_setup();

    // assign S0 and S1 of the shared ram for use by the c28
	// Details of how c28 uses these memory sections is defined
	// in the c28 linker file.
	RAMMReqSharedMemAccess((S1_ACCESS | S2_ACCESS | S4_ACCESS |
	                                    S5_ACCESS), C28_MASTER);

	system_config();

    //  Send boot command to allow the C28 application to begin execution
    //IPCMtoCBootControlSystem(CBROM_MTOC_BOOTMODE_BOOT_FROM_FLASH);

	// Delay
	for (ulLoop=0;ulLoop<500000;ulLoop++){};

	system_init();

	// Enable processor interrupts.
	IntMasterEnable();

	g_ipc_mtoc.ps_module[0].ps_status.bit.model = FBP_DCLink;
	while(1)
	{

	    // TODO: Just when using IHM
	    //g_ipc_mtoc.ps_module[0].ps_status.bit.state = loc_rem_update();

	    switch(g_ipc_mtoc.ps_module[0].ps_status.bit.model)
	    {
	        case FBP:

	            fbp_main();

	        break;

	        case FBP_DCLink:

	            main_fbp_dclink();

            break;

	        //case FAP_DCDC_20kHz:
	        //    fap_dcdc_20_khz_main();
            //
	        //break;
            //
	        //case FAC_Full_DCDC_20kHz:
	        //    fac_full_dcdc_20_khz_main();
            //
	        //break;
            //
	        //case FAC_Full_ACDC_10kHz:
	        //    fac_full_acdc_10_khz_main();
            //
	        //break;
            //
	        //case FAP_ACDC:
	        //    fap_acdc_main();
            //
	        //break;
            //
	        //case FAP_DCDC_15kHz_225A:
	        //    fap_dcdc_15_khz_main();
            //
	        //break;
            //
	        //case FAP_6U_DCDC_20kHz:
	        //    fap_6u_dcdc_20_khz_main();
            //
	        //break;

	        default:
	        break;
	    }
	}

}
