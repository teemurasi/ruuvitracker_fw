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
static WORKING_AREA(usart1_thread, 256);
static WORKING_AREA(usart2_thread, 256);
static WORKING_AREA(usart3_thread, 256);

// Global variables
volatile char usart1_rxbuf[256];
volatile unsigned int usart1_rxbuf_ptr;
volatile char usart2_rxbuf[256];
volatile unsigned int usart2_rxbuf_ptr;
volatile char usart3_rxbuf[256];
volatile unsigned int usart3_rxbuf_ptr;

char welcome[] = {'R','u','u','v','i','T','r','a','c','k','e','r',' ','R','E','V','A',' ','v','0','.','1','\r','\n'};

struct Mutex serial1_tx_mutex;

// Function to send welcome message to USART1
void sendWelcome(void)
{
	uint16_t i = 0;
	//while( chMtxTryLock(&serial1_tx_mutex) != TRUE );
	for( i = 0; i < sizeof(welcome); i++ )
	{
		sdPut(&SD1, welcome[i]);
	}
	//chMtxUnlockAll();
}

// Thread for GPIOC_LED1
static msg_t led1Thread(void *UNUSED(arg))
{
    while(TRUE)
    {
        // Toggle GPIOC_LED1
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
    		if( (usart1_rxbuf[0] == 'P') && (usart1_rxbuf_ptr == 2) )
    		{
				// Initialize and power on the SIM908 module
    			palSetPad(GPIOC, GPIOC_LED2);
				initSIM908();
				SIM908_pwr_on();
				SIM908_autobaud_init();
				chThdSleepMilliseconds(100);
				palClearPad(GPIOC, GPIOC_LED2);
    		}
    		else if( (usart1_rxbuf[0] == 'V') && (usart1_rxbuf_ptr == 2) )
    		{
    			sendWelcome();
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
	uint16_t i = 0;
    while(TRUE)
    {
    	sdRead( &SD2, &byte, 1);
		usart2_rxbuf[usart2_rxbuf_ptr++] = byte;
		if( (byte == '\n') || (usart2_rxbuf_ptr >= 255) )
		{
			while( chMtxTryLock(&serial1_tx_mutex) != TRUE );
			for( i = 0; i < usart2_rxbuf_ptr; i++ )
			{
				sdPut(&SD1, usart2_rxbuf[i]);
			}
			chMtxUnlockAll();
			usart2_rxbuf_ptr = 0;
		}
    }
    return 0;
}

// Thread for USART3
static msg_t usart3Thread(void *UNUSED(arg))
{
    uint8_t byte = 0;
    uint16_t i = 0;
    while(TRUE)
    {
        sdRead( &SD3, &byte, 1);
        usart3_rxbuf[usart3_rxbuf_ptr++] = byte;
		if( (byte == '\n') || (usart3_rxbuf_ptr >= 255) )
		{
			while( chMtxTryLock(&serial1_tx_mutex) != TRUE );
			for( i = 0; i < usart3_rxbuf_ptr; i++ )
			{
				sdPut(&SD1, usart3_rxbuf[i]);
			}
			chMtxUnlockAll();
			usart3_rxbuf_ptr = 0;
		}
    }
    return 0;
}



int main(void)
{
	usart1_rxbuf_ptr = 0;
	usart2_rxbuf_ptr = 0;
	usart3_rxbuf_ptr = 0;

	// Mutex is used to reserve USART1 transmitter for one thread at time
	chMtxInit(&serial1_tx_mutex);

    // Initialize ChibiOS HAL and core
    halInit();
    chSysInit();
    palClearPad(GPIOC, GPIOC_LED2);
    palClearPad(GPIOC, GPIOC_LED1);
    
    // Initialize configuration for USART3 used with SIM908 for AT-commands
    // With autobauding (default) it is only possible to use 57600 baud speed
    SerialConfig serial3_cfg = {
		57600,
		0,
		0,
		0
	};

    // Start serial driver for USART1 using default configuration
    sdStart(&SD1, NULL);
    sendWelcome();
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
    // Start a thread dedicated to handle USART3 communication
    chThdCreateStatic(usart3_thread, sizeof(usart3_thread), NORMALPRIO, usart3Thread, NULL);

    // main loop
    while (TRUE)
    {
	chThdSleepMilliseconds(500);
    }
    return 0;
}
