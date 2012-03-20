#include "sim908.h"
#include "at_v25ter.h"
// ChibiOS specific includes
#include "ch.h"
#include "hal.h"

/*static struct SerialConfig serial_cfg_3 = {
  0, //txend1,
  0, //txend2,
  rxend,
  0, //rxchar,
  0, //rxerr,
  38400,
  0,
  USART_CR2_LINEN,
  0
};*/

char gpsPwrOn[] = {'A','T','+','C','G','P','S','P','W','R','=','1','\r'};
char gpsGetInfo[] = {'A','T','+','C','G','P','S','I','N','F','=','0','\r'};
char gpsGetStatus[] = {'A','T','+','C','G','P','S','S','T','A','T','U','S','?','\r'};
char gpsRstCold[] = {'A','T','+','C','G','P','S','R','S','T','=','0','\r'};

void initSIM908(void)
{
    palSetPad(GPIOC, GPIOC_PWRKEY); // Initial state
    chThdSleepMilliseconds(10);

}

void SIM908_pwr_on(void)
{
    // Use 1sec trigger on PWRKEY pin to start the SIM908 module
    palClearPad(GPIOC, GPIOC_PWRKEY);
    chThdSleepMilliseconds(1000);
    palSetPad(GPIOC, GPIOC_PWRKEY);
}

void sendWithHWFlowCtrl(char *buf, unsigned int bytes)
{
    unsigned int timeout = 5000;
    unsigned char i = 0;
    for( i = 0; i < bytes; i++ )
    {
        while( palReadPad(GPIOB, GPIOB_USART3_CTS) == 0 )
        {
            timeout--;
            if( timeout <= 0 ) break;
        }
        sdPut(&SD3,buf[i]);
    }
}

void SIM908_autobaud_init(void)
{
    unsigned char i = 0;
    for( i = 0; i < 10; i++ )
    {
        sdPut(&SD3,'A');
    }
}

unsigned char SIM908sendCmd(unsigned char cmd)
{
    switch(cmd)
    {
        case GPS_PWR_ON:
            sendWithHWFlowCtrl(gpsPwrOn,sizeof(gpsPwrOn));
            break;
        case GPS_GET_INFO:
        	sendWithHWFlowCtrl(gpsGetInfo,sizeof(gpsGetInfo));
        	break;
        case GPS_GET_STATUS:
			sendWithHWFlowCtrl(gpsGetStatus,sizeof(gpsGetStatus));
			break;
        case GPS_RST_COLD:
        	sendWithHWFlowCtrl(gpsRstCold,sizeof(gpsRstCold));
        	break;
        default:
        break;
    }
    return 1;
}

