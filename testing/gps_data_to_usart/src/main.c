/***
 * Skeleton code for RuuviTracker REVA ChibiOS project
 * by Teemu Rasi 2012, teemu.rasi@gmail.com
 *
 * Modified from chibios-skeleton project originally made by
 * Kalle Vahlman, <zuh@iki.fi>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */
// ChibiOS specific includes
#include "ch.h"
#include "hal.h"

// Project includes
#include "utils.h"
#include "sim908.h"
#include "at_v25ter.h"

// Setup a working area with a 32 byte stack for LED flashing thread
static WORKING_AREA(led_thread, 32);
static WORKING_AREA(led_thread2, 32);
static WORKING_AREA(gps_thread, 256);
static WORKING_AREA(usart1_thread, 256);
static WORKING_AREA(usart2_thread, 256);

// Global variables
volatile char usart1_rxbuf[256];
volatile unsigned int usart1_rxbuf_ptr;

// Thread for GPIOC_LED1
static msg_t led1Thread(void *UNUSED(arg))
{
    while(TRUE)
    {
        // Toggle GPIOC_LED1
        //palTogglePad(GPIOC, GPIOC_LED1);
        //chThdSleepMilliseconds(5000);
		//SIM908sendCmd(110);
		//chThdSleepMilliseconds(5000);
		//SIM908sendCmd(120);
	    // Send one character to serial port
	    //sdPutTimeout(&SD1, 'C', 1);
        // Sleep
        chThdSleepMilliseconds(2000);
        palSetPad(GPIOC, GPIOC_LED1);
        chThdSleepMilliseconds(100);
        palClearPad(GPIOC, GPIOC_LED1);
    }
    return 0;
}

// Thread for GPIOC_LED2
static msg_t led2Thread(void *UNUSED(arg))
{
    while(TRUE)
    {
        // Toggle GPIOC_LED2
        palTogglePad(GPIOC, GPIOC_LED2);
        // Sleep
        chThdSleepMilliseconds(500);
    }
    return 0;
}

// Thread for GPS
static msg_t gpsThread(void *UNUSED(arg))
{
    // TODO: Implement state machine with message queue to set the operating mode
    
    
    // Initialize and power on the SIM908 module
    /*initSIM908();
    SIM908_pwr_on();
    SIM908_autobaud_init();
    chThdSleepMilliseconds(100);
    
    // Send command to power on GPS engine
    palSetPad(GPIOC, GPIOC_LED2);
    SIM908sendCmd(GPS_PWR_ON);
    palClearPad(GPIOC, GPIOC_LED2);
    chThdSleepMilliseconds(5000);
    SIM908sendCmd(GPS_RST_COLD);
    //SIM908sendCmd(110);
    chThdSleepMilliseconds(5000);
    SIM908sendCmd(GPS_GET_STATUS);
    //sdWrite(&SD3,13,"AT+CGPSPWR=1\n");*/
    
    uint8_t byte = 0;
    
    while(TRUE)
    {
        sdRead( &SD3, &byte, 1);
        sdPut(&SD1, byte);
    }
    return 0;
}

// Thread for USART1
static msg_t usart1Thread(void *UNUSED(arg))
{
    // TODO: Implement state machine with message queue to set the operating mode
    
    
	uint8_t byte = 0;
    while(TRUE)
    {
    	// Relay data from usart1 to usart3 by first buffering it untill <cr> received
    	sdRead( &SD1, &byte, 1);
    	usart1_rxbuf[usart1_rxbuf_ptr++] = byte;
    	if( (byte == '\r') || (usart1_rxbuf_ptr >= 255) )
    	{
    		if( (usart1_rxbuf[0] == 'P') )
    		{
				// Initialize and power on the SIM908 module
    			palSetPad(GPIOC, GPIOC_LED2);
				initSIM908();
				SIM908_pwr_on();
				SIM908_autobaud_init();
				chThdSleepMilliseconds(100);
				palClearPad(GPIOC, GPIOC_LED2);
    		}
    		else
    		{
				sendWithHWFlowCtrl((char *)usart1_rxbuf,usart1_rxbuf_ptr);
    		}
    		usart1_rxbuf_ptr = 0;
    	}
    }
    return 0;
}

// Thread for USART2
static msg_t usart2Thread(void *UNUSED(arg))
{
    // TODO: Implement state machine with message queue to set the operating mode


	uint8_t byte = 0;
    while(TRUE)
    {
    	sdRead( &SD2, &byte, 1);
    	sdPut(&SD1, byte);
    }
    return 0;
}

int main(void)
{
	usart1_rxbuf_ptr = 0;

    // Initialize ChibiOS HAL and core
    halInit();
    chSysInit();
    palClearPad(GPIOC, GPIOC_LED2);
    palClearPad(GPIOC, GPIOC_LED1);
    
    // Start serial driver for USART1 using default configuration
    sdStart(&SD1, NULL);
    /*SerialConfig serial2_cfg = {
		115200,
		0,
		0,
		0
	};*/
	SerialConfig serial3_cfg = {
		57600,
		0,
		0,
		0
	};

	// For testing purposes, gps debug is relayed to usart 1

	// Start serial driver for USART2 (SIM908 Debug output and GPS data)
	sdStart(&SD2, NULL);
	// Start serial driver for USART3 (SIM908 serial interface for AT commands)
	sdStart(&SD3, &serial3_cfg);

    // Start a thread dedicated to flashing GPIOC_LED1 and testing serial write
    chThdCreateStatic(led_thread, sizeof(led_thread), NORMALPRIO, led1Thread, NULL);
    // Start a thread dedicated to flashing GPIOC_LED2
    //chThdCreateStatic(led_thread2, sizeof(led_thread2), NORMALPRIO, led2Thread, NULL);
    
    // Start a thread dedicated to handle USART1 communication
    chThdCreateStatic(usart1_thread, sizeof(usart1_thread), NORMALPRIO, usart1Thread, NULL);
    // Start a thread dedicated to handle USART2 communication
    chThdCreateStatic(usart2_thread, sizeof(usart2_thread), NORMALPRIO, usart2Thread, NULL);
    
    // Start a thread dedicated to handle GPS communication
    chThdCreateStatic(gps_thread, sizeof(gps_thread), NORMALPRIO, gpsThread, NULL);

    // main loop
    while (TRUE)
    {
	chThdSleepMilliseconds(500);
    }
    return 0;
}
